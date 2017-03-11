/*############################ fileShareServer.c ############################*/
/**
 * @author Suleyman Balaban - 121044014
 * @date 27 May 2016
 * @version final
 */

#include <stdio.h>                                      //libraries
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
#include <time.h>
#include <dirent.h>
#include <signal.h>

#define BUF_SIZE 4096
#define BACKLOG 4
#define MAX_CHARACTER_SIZE 256

char buf[BUF_SIZE];
char transport[BUF_SIZE];
int clients[50][2];
char transportClient[BUF_SIZE];
int numRead;
int readClient;
int senerr;
char clnt[10];
int port;
char fileCopy[20];

static volatile int doneflag = 0; 
/**
 * This function handle CTRL-C signal
 * @param status
 */
void intHandler(int status);
/**
 * This function make help command
 * @param i key
 */
void help(int i);
/**
 * This function make listServer command
 * @param i key
 */
void listServer(int i);
/**
 * This function make lsClients command
 * @param i key
 * @param clientIndex index of client array
 */
void lsClients(int i,int clientIndex);
/**
 * This function make sendFile command and copyfile
 * @param i key
 * @param status check file is there ?
 */
void sendFileToServer(int i,int status);
/**
 * This function make sendFile  command's message
 * @param message string of message
 * @param size readed byte for write,copy
 * @param file string of file name
 * @return int 1 or 0
 */
int writeToServer(char message[BUF_SIZE],int size,char * file);
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
 * This function delete so clear buf string
 */
void clearBuf();

/*Start Of Main*/
int main(int argc, char *argv[]) {
    int sfd;
    int cfd;
    int b;
    struct sockaddr_in sa;
    FILE *clientsTemp;
    fd_set readfds, masterfds;
    int nready;
    int i,j;
    int maxfd;
    char * temp;
    int clientIndex=-1,keyIndex=-1;
    long timedif=0;
    struct timespec tpend, tpstart;
    time_t current_time;
    char* c_time_string;

    //============================ time ================================
    current_time = time(NULL);

    if (current_time == ((time_t)-1)) {
        (void) fprintf(stderr, "Failure to obtain the current time.\n");
        exit(EXIT_FAILURE);
    }

    c_time_string = ctime(&current_time);

    if (c_time_string == NULL) {
        (void) fprintf(stderr, "Failure to convert the current time.\n");
        exit(EXIT_FAILURE);
    }
    //==================================================================

    signal(SIGINT, intHandler);                     //signal handling
    //""""""""""""""""""""" ARGUMAN """"""""""""""""""""
    if(argc!=2) {
        fprintf(stderr,"USAGE : %s  <port>\n",argv[0]);
        exit(0);
    }
    port=atoi(argv[1]);
    //""""""""""""""""""""""""""""""""""""""""""""""""""
    if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return 1;
    }
    //********************* socket create ************************
    bzero(&sa, sizeof sa);

    sa.sin_family = AF_INET;
    sa.sin_port   = htons(port);

    if (INADDR_ANY)
        sa.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sfd, (struct sockaddr *)&sa, sizeof sa) < 0) {
        perror("bind");
        return 2;
    }

    if (listen(sfd, BACKLOG) == -1)
        return 1;

    FD_ZERO(&masterfds);
    FD_ZERO(&readfds);
    FD_SET(sfd, &masterfds);
    maxfd = sfd;
    //***********************************************************
    fprintf(stdout,"////////////////// WELCOME TO SERVER ///////////////////\n");
    (void) printf("Current time : %s\n", c_time_string);
    /////////////////////////////////////////// MAIN LOOP //////////////////////////////////////////
    for (;;) {
        if(doneflag==1) {
            fprintf(stderr,"\nCTRL-C CATCHED\n");
            break;
        }

        readfds = masterfds;
        nready = select(maxfd + 1, &readfds, NULL, NULL, NULL);

        for (i = 0; i <= maxfd && nready > 0; ++i) {
            if (!FD_ISSET(i, &readfds))
                continue;
            --nready;
            /*bağlantı isteği*/
            if (i == sfd) {
                clientIndex++;
                cfd = accept(sfd, NULL, NULL);
                if (cfd == -1)
                    return 1;
                FD_SET(cfd, &masterfds);
                if (maxfd < cfd)
                    maxfd = cfd;
            }
            /*data*/
            else {
                keyIndex++;
                int snd;
                char dd[256],message[BUF_SIZE],sendClientFile[256];
                clearBuf();

                numRead = read(i, buf, BUF_SIZE);               //read from clients

                if (clock_gettime(CLOCK_REALTIME, &tpstart) == -1) {
                    perror("Failed to get starting time");
                    exit(1);
                }
                //____________________________________ writing to clients__________________________
                if (numRead > 0) {
                    temp=strtok(buf,"*");
                    fprintf(stderr,"--------------------- client : %s -------------------\n",temp);
                    clients[clientIndex][0]=i;
                    clients[clientIndex][1]=atoi(temp);
                    temp=strtok(NULL,"*");
                    fprintf(stderr,"Process => :%s\n",temp);

                    if(strcmp(temp,"help\n")==0) {
                        help(i);
                    }else if(strcmp(temp,"listLocal\n")==0) {
                        fprintf(stderr,"listLocal...\n");
                        write(i, temp, numRead);
                    }else if(strcmp(temp,"listServer\n")==0) {
                        fprintf(stderr,"listServer...\n");
                        listServer(i);
                    }else if(strcmp(temp,"lsClients\n")==0) {
                        fprintf(stderr,"lsClients...\n");
                        lsClients(i,clientIndex);
                    }else if(strstr(temp,"sendFile")!=NULL) {
                        char * copy;
                        int status;

                        strcpy(fileCopy,temp);
                        copy=strtok(fileCopy," &");
                        copy=strtok(NULL," &");
                        fprintf(stderr,"sendFile...\n");
                        strtok(temp,"&");
                        strcpy(message,strtok(NULL,"&"));
                        status=writeToServer(message,atoi(strtok(NULL,"&")),copy);
                        sendFileToServer(i,status);
                    }else if(strcmp(temp,"Exit")==0){
                        write(i,temp,strlen(temp));
                    }else
                        help(i);                            //if wrong commands come
                    clearBuf();
                    if (clock_gettime(CLOCK_REALTIME, &tpend) == -1) {
                        perror("Failed to get ending time");
                        exit(1);
                    }
                    timedif = 1000000*(tpend.tv_sec - tpstart.tv_sec) + (tpend.tv_nsec - tpstart.tv_nsec)/1000;
                    printf("mikroseconds :%ld\n",timedif );
                    fprintf(stderr,"------------------------------------------------------\n");
                }
                else {
                    close(i);
                    FD_CLR(i, &masterfds);
                }
                //______________________________________________________________________________________
            }
        }
    }
    ///////////////////////////////////////////////////////////////////////////////////////////
    puts("\nAll data saved...");
    puts("Terminated successfuly");
    fprintf(stdout,"\n//////////////////////// THE END  ///////////////////////\n");
    return 0;
}
/*End Of Main*/
/*------------------------------------------------------------------------*/
void help(int i) {
    char helpString[256];
    sprintf(helpString,"\n1-) listLocal\n2-) listServer\n3-) lsClients\n4-) sendFile <filename>\n5-) help\n");
    write(i,helpString,strlen(helpString));
    strcpy(helpString,"\0");
}
/*------------------------------------------------------------------------*/
void listServer(int i) {
    char cwd[256];
    char fileNames[1000][256];
    int j,numberOfFiles=0;
    char tempFileName[80];
    clearBuf();

    if ((getcwd(cwd, sizeof(cwd))) == NULL)
        perror("getcwd() error");
    readDirectory(cwd,fileNames,&numberOfFiles);
    for (j = 0; j <numberOfFiles; ++j) {
        sprintf(tempFileName,"%s\n",fileNames[j]);
        strcat(buf,tempFileName);
    }
    write(i,buf,strlen(buf));
}
/*------------------------------------------------------------------------*/
void lsClients(int i,int clientIndex) {
    int j;
    char sendClients[256];
    char tempPid[10];

    clearBuf();
    for (j = 0; j < clientIndex+1; ++j) {
        sprintf(tempPid,"%d\n",clients[j][1]);
        if(j==0)
            strcpy(sendClients,tempPid);
        else
            strcat(sendClients,tempPid);
    }
    write(i,sendClients,strlen(sendClients));
}
/*------------------------------------------------------------------------*/
void sendFileToServer(int i,int status) {
    char sendClients[256];
    if(status==1)
        strcpy(sendClients,"sended file to server!");
    else
        strcpy(sendClients,"Couldn't copy file.There is no that file!");
    write(i,sendClients,strlen(sendClients));
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
        buf[j]='\0';
    }
}
/*------------------------------------------------------------------------*/
int writeToServer(char message[BUF_SIZE],int size,char * file) {
    FILE * filewrite;
    char filer[20];

    if(size!=-1) { 
        strcpy(filer,"COPY");
        strcat(filer,file);
        filewrite=fopen(filer,"wb+");
        fwrite(message,1,size,filewrite);
        fclose(filewrite);
        return 1;
    }else {
        fprintf(stderr,"There is no file have that name!\n");
        return 0;
    }
}
/*------------------------------------------------------------------------*/
int find(int num) {
    int i;
    for (i = 0; i<2; ++i) {
        if(clients[i][1]==num) {
            return clients[i][0];
        }
    }
    return -999;
}
/*------------------------------------------------------------------------*/
void intHandler(int status) {
    doneflag = 1;
}
/*############################## END OF PROGRAM ##############################*/