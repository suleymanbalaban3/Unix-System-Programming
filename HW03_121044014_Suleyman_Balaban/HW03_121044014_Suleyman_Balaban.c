/**
 * @author Suleyman Balaban - 121044014
 * @date 5 APRIL 2016
 * @version 3
 * @link https://linuxprograms.wordpress.com/2008/02/15/programming-with-fifo-mkfifo-mknod/
 * @description This is find occuarence in directories by given string.
 * Display the line and column number to "gdf.log" file and print the sum of 
 * occuarence for all files
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

#define FIFO_PERM (S_IRUSR | S_IWUSR)
//global variable for sum of occuarence in al files
#define SIZE 256 //maksimum dosya ismi boyutu
static volatile sig_atomic_t exitProgram = 0;//for signal handling
typedef enum{FALSE=0,TRUE=1}BOOL;//typedef for BOOL type
/**
 * This function make all processing about program,for one file 
 * return occuarence number and print to output file lines and columns
 * @param inputFile pointer of the input file
 * @param fileName pointer of the input file
 * @param target will find in file this string
 * @param outputFile file pointer for print the results to file
 * @return int target's occuarence in file 
 */
int grepFromFile(FILE *inputFile,char *fileName,char *target,FILE *outputFile,int fildes);
/**
 * This function make process,call grepFromFile function for each files 
 * and print sum of occuarence for all files in outerForRes
 * @param directoryName directory name
 * @param target will find in file this string
 * @param outputFile file pointer for print the results to file
 * @param flag for malloc it will increase for when each file read
 * @param counterFile file pointer for print occuarences
 */
void grepFromDirectory(char *directoryName,char *target,FILE *outputFile,int flag,int* total);
/**
 * This function check is directory or .txt 
 * @param directoryName directory name
 * @return BOOL if txt: TRUE,if directory :FALSE return 
 */
BOOL isTxt(char *directoryName);
/**
 * This function count how many directory have 
 * @param directory directory name to start position
 * @param dirCount integer pointer to count directories
 */
void dirCounter(const char *directory,int * dirCount);
/**
 * This function manage the signal
 * @param signal integer signal input
 */
static void ex_program(int signal) { exitProgram = 1; }

//Start Of Main

int main(int argc,char *argv[]) {
    FILE *outputFile;//file pointer gfd.log,outerForRes
    int flag=0;//for using recursive grepfromDirectory function  
    pid_t childpid;
    int openFifo;
    char * fifoName="fifoName";
    int toplam=0,val;
    char *buf;
    int total=0,allOcc=0;
	int dirCount=1,temp=0,i=0;
	int result;//for mkfifo control

	result=mkfifo(fifoName,0666);
	if(result!=0) {
		perror ("fifo couldn't open!\n");
        exit(0);
	}
	dirCounter(argv[1],&dirCount);//directory count
	
    if(argc!=3) {//arguman check
        fprintf(stderr,"Usage : %s [directory] [target]\n",argv[0]);
        exit(0);
    }//open and send to after checked function 
	if((outputFile=fopen("gfd.log","a"))==NULL) {
	    printf("Dosya Hatasi : %s\n",strerror(errno));
	    exit(0);
	} 
	signal(SIGINT, ex_program);
    childpid=fork();
    if(childpid==0)
        grepFromDirectory(argv[1],argv[2],outputFile,flag,&total);
    else {
        openFifo=open(fifoName,O_RDONLY);
        printf("------------- Occurrences For Each Directory -------------\n");
        while(i<dirCount) {   
			sleep(2);
			if(exitProgram==1) {
				fprintf(stderr,"\nExiting from the program...\n");
				break;
			}
			buf=(char*)malloc(80*sizeof(char));
            read(openFifo,buf,80);
			toplam=temp;
            printf("%s\n",buf );
			strtok(buf,":");
			allOcc+=atoi(strtok(NULL,":"));
			free(buf);		
            ++i;
        }
		unlink(fifoName);
		close(openFifo);
		fclose(outputFile);
        printf("--------------------- All Occurrence ---------------------\n");
        printf("Result :%d\n",allOcc);
    }
    return 0;
}//End Of Main
/*##################################################################################################*/
void grepFromDirectory(char *directoryName,char *target,FILE *outputFile,int flag,int* total) {  

    DIR *dirPointer;//directory pointer
    struct dirent *dirInfo;
    FILE *inputFile;
    int occuarence=0;
    char *fileName;
    pid_t pid;
    int fd[2];
    int openFifo;
    char * fifoName="fifoName";
    char *fifoString;
    int temp;

    dirPointer=opendir(directoryName);//open directory
    if (dirPointer==NULL) {
        perror("open directory");
        exit(1);
    }
    if (pipe(fd) == -1) {
        perror("Error in pipe. Terminated.");
        exit(1);
    }
    while ((dirInfo=readdir(dirPointer)) != NULL) {
        if(strcmp(dirInfo->d_name,".")!=0 && strcmp(dirInfo->d_name,"..")!=0) {
            flag++;//malloc for each file increase this because of recursive
            fileName=(char*)malloc(flag*SIZE*sizeof(char));
            if(fileName==NULL) {
                fprintf(stderr,"Memory error\n");
                exit(0);
            }
            strcpy(fileName,directoryName);//add tools
            strcat(fileName,"/");
            strcat(fileName,dirInfo->d_name);
            if(isTxt(fileName)==TRUE) {//if filename is txt
                if ((pid = fork()) == -1) {
                   perror("fork error :\n");
                   exit(1);
                }
                if(pid == 0) {//add occurrences to all allOccuarence and print outerForRes.txt
                    close(fd[0]);
                   grepFromFile(inputFile,fileName,target,outputFile,fd[1]);
                   close(fd[1]);
                   exit(0);
                }
                else {
                    //while(wait(NULL)!=-1);    
                    close(fd[1]);
					read(fd[0],&temp,sizeof(int));
					*total+=temp;
                    close(fd[0]); 
                }
				if ((openFifo = open(fifoName, O_WRONLY)) < 0)
						perror("child - open");	
					fifoString=(char*)malloc(80*sizeof(char));			
					sprintf(fifoString,"%s => :%d\n",directoryName,*total);
					*total=0;
					write(openFifo,fifoString,80);
					free(fifoString);
					close(openFifo);
           	    }
            else{//if directory : call self so,recursive
                grepFromDirectory(fileName,target,outputFile,flag,total);    
            }    
            free(fileName);//free dinamic filenName string
        }
    }
    closedir(dirPointer);//close directory
}
/*###############################################################################*/
int grepFromFile(FILE *inputFile,char *fileName,char *target,FILE *outputFile,int fildes) {

    int targetSize=strlen(target)+1;
    int letter=0;
    int row=1,column=1;
    int occuarence=0;
    char * foundedTarget=(char*)malloc(strlen(target)*sizeof(char));

    if((inputFile=fopen(fileName,"r"))==NULL) {//if input file couldn't open
        printf("Dosya Hatasi 2 %s: %s\n",fileName,strerror(errno));
        exit(0);
    }
    if(foundedTarget==NULL) {//memory control
        fprintf(stderr,"Memory error\n");
        exit(0);
    }
    fprintf(outputFile,"#%s\n   \tLine      Column\n",fileName);
    while(fgets(foundedTarget,targetSize,inputFile)!=NULL) {
        letter++;//for input,fseek()
        if(strcmp(foundedTarget,"\n")==0) {//if new line character
            row++;//row number increase +1
            column=0;//column number not change in new line
        }
        if(strcmp(foundedTarget,target)==0) {
            occuarence++;//occuarence in file if equals target and founded
            fprintf(outputFile,"%d.\t  %d     \t%d\n",occuarence,row,column);
        }
        column++;
        fseek(inputFile,letter,SEEK_SET);
    }  
    if(occuarence==0) {
        fprintf(outputFile,"  !!!There is no \"%s\" in this file\n",target);
    }
    write(fildes,&occuarence,sizeof(int));
    fclose(inputFile);//close the file
    free(foundedTarget);//free string

    return (occuarence);//return occuarence in file
}
/*##############################################*/
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
void dirCounter(const char *directory,int * dirCount)	
{
    DIR *dir;
    struct dirent *dirp;
	char *string;

    if (!(dir = opendir(directory)))
        return;
	while((dirp = readdir(dir)) != NULL)
	{
		if(strcmp(dirp->d_name, ".") != 0 && strcmp(dirp->d_name, "..") != 0)
		{
			string=(char*)malloc(SIZE*sizeof(char));
			strcpy(string,directory);		/* dosyayi acabilmesi icin doyanin */
			strcat(string,"/");			/* bulundugu konumu isme ekledim */
			strcat(string,dirp->d_name);
					
			if (isTxt(string)==FALSE){
				++*dirCount;				/*acilan alt directory'leri saydim*/
	       		dirCounter(string,dirCount);
	    	}
			free(string);
		}
	} 
    closedir(dir);
}
/*********************** END OF HW03_121044014_Suleyman_Balaban.c *********************/
