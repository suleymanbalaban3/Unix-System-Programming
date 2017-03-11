/**
 * @author Suleyman Balaban - 121044014
 * @date 5 May 2016
 * @version 6
 * @link https://www.cs.cf.ac.uk/Dave/C/node32.html
 * @link System Programming Lesson Book
 * @description This is find occuarence in directories by given string.
 * Display the line and column number to "gfdth.log" file and print the sum of 
 * occuarence for all files using fork,thread,semaphore,shared memory,messagge queue
 */
#include <stdio.h>//libraries
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/ipc.h>
#define MSGSZ     128

typedef struct msgbuf {
   long    mtype;
   int mtext;
} message;

#define SHMSIZE 256
#define MAX_CHARACTER_SIZE 256
#define WORDS "gfdt.log"
#define MAX_SIZE 1000
#define MAX_SIZE_RESULT 50
#define SIZE 256 //maksimum dosya ismi boyutu
static volatile sig_atomic_t exitProgram = 0;

sem_t sem;

typedef enum{FALSE=0,TRUE=1}BOOL;

/**
 * This function count directory number
 * @param directory directory name
 * @param dirCount return number pointer
 */
void dirCounter(const char *directory,int * dirCount) ;
/**
 * This function check is directory or .txt 
 * @param directoryName directory name
 * @return BOOL if txt: TRUE,if directory :FALSE return 
 */
BOOL isTxt(char *directoryName);
/**
 * This function make process,call grepFromFile function for each files 
 * and print sum of occuarence for all files in outerForRes
 * @param directoryName directory name
 * @param target will find in file this string
 */
void grepFromDirectory(char *directoryName,char *target,int *occurrence);
/**
 * This function find occurrences for one file
 * @param fileName file name
 * @return int return occrurrence
 */
int grepFromFile(char *fileName);
/**
 * This function create thread for each files in current directory 
 * @param dirName directory name
 * @param target target word
 * @param result string about directory informations
 */
void threadForFiles(char *dirName,char *target,int * occurrence);
/**
 * This function find first files in it
 * @param path directory name
 * @param fileNames filenames in it this directory
 * @param fileSize return number pointer
 * @return int 1 succes
 */
int findFile(char *path,char fileNames[][256], int *fileSize);
/**
 * This function find all files in it and make thread
 * @param tt thread array
 * @param path directory name
 * @param fileNames filenames in it this directory
 * @param target target word
 * @param fileSize file counter
 * @param dirSize directory counter
 * @return int 1 succes
 */
int readDirectory(pthread_t tt[],char *path,char fileNames[][MAX_SIZE],char *target,int *fileSize, int *dirSize);
/**
 * This function read files using thread
 * @param arr void * to thread reading
 */
void* readWithThread(void *arr);
/**
 * This function check directory or file
 * @param arr void * to thread reading
 */
int isdirectory(char *path);
/**
 * This function handle CTRL-C signal
 * @param signal 
 */
static void ex_program(int signal) { exitProgram = 1; }
/*##################################################################################################*/
//Start Of Main
int main(int argc,char *argv[]) {
    pid_t childpid;
    int i=0,j=0,count=0;
    int dirCount=1;
    int allOcc=0;
    int shmid;
    char *shm;
    key_t key;
    int occurrence=0;
    char number[3]={};

    if(argc!=3) {//arguman check
        fprintf(stderr,"Usage : %s [directory] [target]\n",argv[0]);
        exit(0);
    }//open and send to after checked function   
    dirCounter(argv[1],&dirCount); 
    signal(SIGINT, ex_program);

    if(sem_init(&sem,0,1)==-1) {
        fprintf(stderr,"Couldn't create semaphore\n");
        exit(1);
    }
    if((shmid = shmget(1453, SHMSIZE,IPC_CREAT|0666))<0) {
        perror("hata share !\n");
        exit(1);
    }
    if ((childpid = fork()) == -1) {
       perror("fork error :\n");
       exit(1);
    }
    if(childpid==0) {
        grepFromDirectory(argv[1],argv[2],&occurrence);
        if((shm = (char *)shmat(shmid, NULL,0))==(void *)-1) {
            perror("shamet1 error !\n");
            exit(1);
        }
        strcat(shm,"\0");
        shmdt(shm);
    }
    else {
        wait(NULL);
        puts("\n############################ WELCOME ##############################\n");
        printf("------------- Occurrences For Each Directory -------------\n");
        if((shm = (char *)shmat(shmid, NULL,0))==(void *)-1) {  // shared memory read
            perror("shamet2 error !\n");
            exit(1);
        }
        for (i = 0;shm[i]!='\0'; ++i) {
            if(shm[i]==':') {
                sleep(1);
                if(exitProgram==1) {
                    fprintf(stderr,"Exitting !\n");
                    break;
                }
                for (j = i;shm[j+1]!='\n'; ++j) {
                    number[count]=shm[j+1];
                    count++;
                }
                allOcc+=atoi(number);   
                for (j = 0; j < 4; ++j) {
                    number[j]='\0';
                }
                count=0;
            }
            printf("%c",shm[i]);
        }
        printf("\n");
        shmdt(shm);
        shmctl(shmid,IPC_RMID,NULL);
        printf("--------------------- All Occurrence ---------------------\n");
        printf("Result of all occurrences :%d\n",allOcc);
        printf(".\n.\n.\n");
        fprintf(stderr,"Data saved successfuly... \n");
        puts("\n############################ THE END ##############################\n");
    }
    return 0;
}//End Of Main
/*##################################################################################################*/
void grepFromDirectory(char *directoryName,char *target,int *occurrence) {  
    pid_t pid;
    int status,numberOfFiles=0;
    char fileNames[1000][256];
    char string[80];
    int i;
    int shmid;
    char *shm;
    int msqid;
    key_t key;
    message sbuf;
 
    status=findFile(directoryName,fileNames,&numberOfFiles);    //find dir count,files

    for (i = 0; i < 1; ++i)
        if((pid=fork())<=0)                 //fan fork for each directory
            break;
    if(pid==0) {
        for(i = 0; i < numberOfFiles; ++i) {
            if (isdirectory(fileNames[i]))
                grepFromDirectory(fileNames[i],target,occurrence); //recursive at new directory
        }
        threadForFiles(directoryName,target,occurrence);

        while (sem_wait(&sem) == -1)                            //semafor

        key=getppid();

        if ((msqid = msgget(key, IPC_CREAT | 0666 )) < 0) {     //messsage queue
            perror("msgget ,,");
            exit(1);
        }
        sbuf.mtype = 1;
        sbuf.mtext=*occurrence;
        if (msgsnd(msqid, (void *)&sbuf, sizeof(sbuf), IPC_NOWAIT) < 0) {//messagge write
            perror("msgsnd");
            exit(1);
        }

        if (sem_post(&sem) == -1)
            fprintf(stderr, "Thread failed to unlock semaphore\n");
        exit(1);
    }else {
        while(wait(NULL)!=-1);

        while (sem_wait(&sem) == -1)
        key=getpid();
        if ((msqid = msgget(key, IPC_CREAT | 0666 )) < 0) {
            perror("msgget ,,");
            exit(1);
        }
        if (msgrcv(msqid,&sbuf,sizeof(sbuf), 1, 0) < 0) {  //messagge read
            perror("msgrcv");
            exit(1);
        }
        if (sem_post(&sem) == -1)
            fprintf(stderr, "Thread failed to unlock semaphore\n");
        sprintf(string,"######### %s ########\nNumber of occurrence :%d\n",directoryName,sbuf.mtext);
        if((shmid = shmget(1453, SHMSIZE,IPC_CREAT|0666))<0) {
            perror("hata share !\n");
            exit(1);
        }
        if((shm = (char *)shmat(shmid, NULL,0))==(void *)-1) {  // shared memory write
            perror("shamet3 error !\n");
            exit(1);
        }
        strcat(shm,string);
        shmdt(shm);
    }
}
/*##################################################################################################*/
void threadForFiles(char *dirName,char *target,int *occurrence)
{
    int i,status,wordCount=0,numberOfDirectories=-1,numberOfFiles=0;
    char fileNames[MAX_SIZE][MAX_SIZE],*word;
    pthread_t tid[MAX_SIZE];
    char *token;
    char result[50];

    if(dirName[strlen(dirName)-1]=='/' && strlen(dirName)!=1)
        dirName[strlen(dirName)-1]='\0';
    status=readDirectory(tid,dirName,fileNames,target,&numberOfFiles,&numberOfDirectories);
    if(status==-1)
        return;
    for (i = 0; i < numberOfFiles; ++i) {
        if (pthread_join(tid[i], (void **)&word))
            perror("Failed to join thread");
        token = strtok(word," ");
        wordCount+= atoi(token);

    }
    *occurrence=wordCount;
}
/*##################################################################################################*/
int readDirectory(pthread_t tt[],char *path,char fileNames[][MAX_SIZE],char * target,int *fileSize, int *dirSize) {
    struct dirent *direntp,*direntp2;
    DIR *dirp,*dirp2;
    char dirName[MAX_SIZE];
    char cwd[MAX_SIZE];

    if(isdirectory(path)>0) {
        ++*dirSize;
    }
    else if(isdirectory(path)==0) { 
        // find current directory
        getcwd(cwd, sizeof(cwd));
        strcat(cwd,"/");
        strcat(cwd,WORDS);
        if(strcmp(path,cwd)!=0) { // for not being infinite loop
            strcpy(fileNames[*fileSize],target);
            strcat(fileNames[*fileSize]," ");
            strcat(fileNames[*fileSize],path);
            if (pthread_create(tt+*fileSize, NULL, readWithThread, fileNames[*fileSize]))
                perror("Failed to create thread");
            ++(*fileSize);
            if(*fileSize==MAX_SIZE) {
                fprintf(stderr, "Maximum size (%d)! \n",MAX_SIZE);
                exit(1);
            }
        }
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
        if(!(isdirectory(dirName)>0)) {
            readDirectory(tt,dirName,fileNames,target,fileSize,dirSize);
        }
        
        
    }

    while ((closedir(dirp) == -1) && (errno == EINTR));
    return 1;
}
/*##################################################################################################*/
int grepFromFile(char *fileName) {
    FILE * input,* output;//file pointers
    char foundedTarget[100];//readed string from file
    char * file;
    int row=1,column=1,letter=0,occuarence=0;
    char * target=strtok(fileName," ");
    file=strtok(NULL," ");
    int  targetSize=strlen(target)+1;//target string's size

    input = fopen(file,"r");
    if(input==NULL) {
        perror("Failed to open file");
        return -1;
    }

    output = fopen(WORDS,"a");
    if(output==NULL) {
        fclose(input);
        return -1;
    }
    if(fileName[0]=='.') {
        fclose(input);
        fclose(output);
        return -1;
    }
    if(output==NULL) {//if output file couldn't open
        fprintf(stderr,"Output File couldn't open!!\n");
        exit(1);
    }
    if(input==NULL) { //if input file couldn't open
        fprintf(stderr,"File can't open!\n");
        exit(1);
    }   
    fprintf(output,"#%s\n",file);
    fprintf(output,"   \tLine      Column\n");
    while(fgets(foundedTarget,targetSize,input)!=NULL) {//read stringd
        letter++;//for input,fseek()
        if(strcmp(foundedTarget,"\n")==0) {//if new line character
            row++;//row number increase +1
            column=0;//column number not change in new line
        }
        if(strcmp(foundedTarget,target)==0) {
            occuarence++;//occuarence in file if equals target and founded
            fprintf(output,"%d.\t  %d     \t%d\n",occuarence,row,column);
        }
        fseek(input,letter, SEEK_SET);//
        column++;
    }
    if(occuarence==0)
        fprintf(output,"  !!!There is no \"%s\" in this file\n",target);
    fclose(input);//close the files
    fclose(output);
    return occuarence;//return occuarence in file
}
/*##################################################################################################*/
BOOL isTxt(char *directoryName) {
    DIR *directoryPointer;
    directoryPointer=opendir(directoryName);//open directory
    if (directoryPointer==NULL) {//if txt
        closedir(directoryPointer);
        return TRUE;
    }
    closedir(directoryPointer);//else directory
    return FALSE;
}
/*##################################################################################################*/
int findFile(char *path,char fileNames[][256], int *fileSize) {
    struct dirent *direntp,*direntp2;
    DIR *dirp,*dirp2;
    DIR *dirPointer;
    struct dirent *dirInfo;
    char dirName[MAX_CHARACTER_SIZE];
    char fileName[256];
    int i=0;
    dirPointer=opendir(path);
    
    if (dirPointer==NULL) {
        perror("open directory");
        exit(1);
    }
    else if(isdirectory(path)==0) {
        strcpy(fileNames[*fileSize],path);
    }
    while ((dirInfo=readdir(dirPointer)) != NULL) {      
        if(strcmp(dirInfo->d_name,".")!=0 && strcmp(dirInfo->d_name,"..")!=0) {
            if(fileName==NULL) {
                fprintf(stderr,"Memory error\n");
                exit(0);
            }
            strcpy(fileName,path);//add tools
            strcat(fileName,"/");
            strcat(fileName,dirInfo->d_name);
            strcpy(fileNames[i],fileName);
            i++;
            ++*fileSize;
        }
    }
    closedir(dirPointer);
    return 1;
}
/*##################################################################################################*/
void dirCounter(const char *directory,int * dirCount)   
{
    DIR *dir;
    struct dirent *dirp;
    char string[256];

    if (!(dir = opendir(directory)))
        return;
    while((dirp = readdir(dir)) != NULL)
    {
        if(strcmp(dirp->d_name, ".") != 0 && strcmp(dirp->d_name, "..") != 0)
        {
            strcpy(string,directory);       /* dosyayi acabilmesi icin doyanin */
            strcat(string,"/");         /* bulundugu konumu isme ekledim */
            strcat(string,dirp->d_name);
                    
            if (isTxt(string)==FALSE){
                ++*dirCount;                /*acilan alt directory'leri saydim*/
                dirCounter(string,dirCount);
            }

        }
    } 
    closedir(dir);
}
/*##################################################################################################*/
int isdirectory(char *path) {
    struct stat statbuf;

    if (stat(path, &statbuf) == -1)
        return -1;
    else
        return S_ISDIR(statbuf.st_mode);
}
void* readWithThread(void *arr) {

    while (sem_wait(&sem) == -1)
    if(errno != EINTR) {
        fprintf(stderr, "Thread failed to lock semaphore\n");
        return NULL;
    }
    int numberOfWord = grepFromFile((char *)arr);
    if (sem_post(&sem) == -1)
        fprintf(stderr, "Thread failed to unlock semaphore\n");
    sprintf(arr,"%d ",numberOfWord);
    return arr;
}
/*********************** END OF grepFromDirSemaphore.c *********************/