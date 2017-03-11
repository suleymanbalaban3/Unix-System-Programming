/*############################## client.c ##############################*/
/**
 * @author Suleyman Balaban - 121044014
 * @date 27 May 2016
 * @version final
 */

#include <stdio.h>                                  //libraries
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <signal.h>

#define BUF_SIZE 4096
#define MAX_CHARACTER_SIZE 256

char buf[80];
char transport[BUF_SIZE];
char result[4096];
char sendClient[256];

static volatile int doneflag = 0; 
/**
 * This function handle CTRL-C signal
 * @param status
 */
void intHandler(int status);
/**
 * This function list local files in client's directory
 */
void listLocal();
/**
 * This function send which file will copy 
 */
void sendFile();
/**
 * This function check string is directory?
 * @param path string of file name
 * @return int 1 or 0
 */
int isdirectory(char *path);
/**
 * This function read and find files in local directory
 * @param path string of file name
 * @param filenames two dimensional string for store file names in it
 * @param fileSize file number
 * @return int 1 or 0
 */
int readDirectory(char *path,char fileNames[][256], int *fileSize);
/**
 * This function delete so clear transport string
 */
void clearBuf();
/**
 * This function read which files entered to copy 
 * @param fileName string of file name
 * @return int byte number or -1
 */
int readFile(char fileName[20]);

/*Start Of Main*/
int main(int argc, char **argv) {
    int sfd;
    int bytes;
    struct sockaddr_in sa;
    struct sigaction act;
    ssize_t numRead;
    pid_t pid;

    act.sa_handler = intHandler;                        //signal
    act.sa_flags = 0;
    if ((sigemptyset(&act.sa_mask) == -1) || (sigaction(SIGINT, &act, NULL) == -1))
    {
        perror("Failed to set SIGINT handler");
        return 1;
    };  
    if (argc != 3) {                    
        printf("usage: %s <IP adresi> <port>\n",argv[0]);
        return 1;
    }
    //______________________________________________________________________________
    printf("################## Client Id :%d ##################\n",getpid());
    if ((sfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return 1;
    }

    bzero(&sa, sizeof sa);

    sa.sin_family = AF_INET;
    sa.sin_port = htons(atoi(argv[2]));
    sa.sin_addr.s_addr = inet_addr(argv[1]);

    if (connect(sfd, (struct sockaddr *)&sa, sizeof sa) < 0) {
        perror("connect");
        close(sfd);
        return 2;
    }
    //______________________________________________________________________________
    for (;;) {
        if((pid=fork())<=0)
            break;
        else{
            char *token;
            int count=0;
            char sendFile[256],fileName[20],temp[10],sendClientPid[10];

            puts("__________________________________________________________");
            puts("__________________ send message to server ________________");
            puts("__________________________________________________________");
            fprintf(stderr,"Message =>:");
            fgets(buf, 80, stdin);
            if(doneflag==1) {
                fprintf(stderr,"\nCTRL-C CATCHED !\n");
                sprintf(transport,"%d*%s",getpid(),"Exit");
                //break;
            }else{
                sprintf(transport,"%d*%s",getpid(),buf);
                //________________________________sendFile_______________________________________
                if(strstr(buf,"sendFile")!=NULL){           //if request sendFile
                    token=strtok(buf," ");
                    strcpy(sendFile,token);
                    while(token != NULL) {
                        printf( " %s\n", token );
                        token = strtok(NULL, " ");
                        if(count==0)
                            strcpy(fileName,token);
                        count++;
                    }
                    if(count>3) {
                        fprintf(stderr,"You entered wrong input for sendFile !\n");
                        fprintf(stderr,"\tsendFile <filename> <client id>\n");
                        fprintf(stderr,"\t\tor\n\tsendFile <filename>\n");
                        exit(1);
                    }else {
                        sprintf(temp," %d",readFile(fileName));
                        strcat(sendFile," ");
                        strcat(sendFile,fileName);
                        strcat(transport,"&");
                        strcat(transport,result);
                        strcat(transport,"&");
                        strcat(transport,temp);
                    }
                }
                //______________________________________________________________________
            }
            if (write(sfd, transport, strlen(transport)) < 0) {         //write to server
                return 1;
            }
            clearBuf();                                                 //clear transport string
            if(numRead = read(sfd, transport, BUF_SIZE) < 0) {          //read from server
                break;
            }
            if(strcmp(transport,"Exit")==0) {
                fprintf(stderr,"exitt !\n");
                fprintf(stderr,"##################### THE END ####################\n");
                break;
            }
            puts("----------- getting message from server -----------");
            fprintf(stderr,".\n.\n.\n");
            puts(transport);
            if(strcmp(transport,"listLocal\n")==0) {
                listLocal();
            }
            clearBuf();
        }
    }
    //_________________________________________________________________________________
    close(sfd);
    return 0;
}
/*End Of Main*/
/*------------------------------------------------------------------------*/
void listLocal() {
    char cwd[256];
    char fileNames[1000][256];
    int i,numberOfFiles=0;
    if ((getcwd(cwd, sizeof(cwd))) == NULL)
        perror("getcwd() error");
    readDirectory(cwd,fileNames,&numberOfFiles);
    for (i = 0; i <numberOfFiles; ++i) {
        fprintf(stderr,"%s\n",fileNames[i]);
    }
}
/*------------------------------------------------------------------------*/
int isdirectory(char *path) {
    struct stat statbuf;

    if (stat(path, &statbuf) == -1)
        return -1;
    else
        return S_ISDIR(statbuf.st_mode);
}
/*------------------------------------------------------------------------*/
int readDirectory(char *path,char fileNames[][256], int *fileSize) {
    struct dirent *direntp,*direntp2;
    DIR *dirp,*dirp2;
    char dirName[MAX_CHARACTER_SIZE];

    if(isdirectory(path)>0) {
        strcpy(fileNames[*fileSize],path);
        ++*fileSize;
    }
    else if(isdirectory(path)==0) {
        strcpy(fileNames[*fileSize],path);
        ++*fileSize;
    }
    else {
        printf("Error! %s is not a directory!\n",path );
        return -1;
    }
    if ((dirp = opendir(path)) == NULL || path[0]=='.') 
        return 0;
    while ((direntp = readdir(dirp)) != NULL) {
        if(direntp->d_name[0]=='.')
            continue;
        strcpy(dirName,path);
        strcat(dirName,"/");
        strcat(dirName,direntp->d_name);
        readDirectory(dirName,fileNames,fileSize);
    }
    while ((closedir(dirp) == -1) && (errno == EINTR));
    return 1;
}
/*------------------------------------------------------------------------*/
void clearBuf() {
    int j;
    for (j = 0; j < BUF_SIZE; ++j) {
        transport[j]='\0';
    }
}
/*------------------------------------------------------------------------*/
int readFile(char fileName[20]) {
    int size=100;
    char * file;

    file=strtok(fileName,"\n");
    int fileread = open(file, O_RDONLY);
    if(fileread==-1) {
        strcpy(result,"nothing");
        return -1;
    }
    void *buffer;
    buffer = malloc(sizeof(void) * size);

    int nread = read(fileread,buffer,size);
    strcpy(result,(char*)buffer);

    free(buffer);
    close(fileread);
    
    return nread;
}
/*------------------------------------------------------------------------*/
void intHandler(int status) {
    doneflag=1;
}
/*############################## END OF PROGRAM ##############################*/