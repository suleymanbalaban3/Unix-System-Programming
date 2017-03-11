/**
 * @author Suleyman Balaban - 121044014
 * @date 20 March 2016
 * @version vize
 * @link http://www.thegeekstuff.com/2012/03/c-process-control-functions/
 */

#include <errno.h>									//libraries
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>
#include <sys/wait.h>
#include <string.h>
#include <math.h>

#define FIFO_PERMS (S_IRWXU | S_IWGRP| S_IWOTH)		//defines
#define BLKSIZE PIPE_BUF
#define FILE_NAME_SIZE 256
#define FIFONAME "slymn"
//glabal variable
static volatile int doneflag = 0;					//global variables
/**
 * This function check arguman file names is true or false
 * @param arr arguman string
 * @return int 1 or 0
 */
int argumanCheck(char arr[10]);
/**
 * This function check arguman operand names is +,-,*,/
 * @param arr arguman string
 * @return int 1 or 0
 */
int operandCheck(char arr[10]);
/**
 * This function catch and handle c-process-control-functions (CTRL-C)
 * @param status 
 */
void intHandler(int status); 
/////////////////////////////////////////////////////////////////////////////////////////////////
int main (int argc, char *argv[]) {
    FILE *clientCounter;								//client count file 
    FILE *logOutput;;									//.log file pointer
	int mainFifo,clientFifo;							//to open and mkfifo functions
	char string[256];									//to send argumans using a this string
	char newString[256];								//to read results 
    char clientPid[10];									//to read results 
    char logFileName[15];								//print results clients in this file
    int i,count;										/*count client sayisini 1 azaltmak icin*/

    signal(SIGINT, intHandler);							/*signal handler*/
    
	if(argc < 5) {
		fprintf(stderr,"Usage: %s -<leftFunc> -<rightFunc> -<interval> -<operator>...\n",argv[0]);
		return 1;
	}
	for (i = 1; i < 4; ++i) {
		if(argv[i][0] != '-') {
			fprintf(stderr,"You must add '-' each command.\n");
			fprintf(stderr,"Usage: %s -<leftFunc> -<rightFunc> -<interval> -<operator>...\n",argv[0]);
			return 1;
		}
	}
	if ((mainFifo = open(FIFONAME, O_WRONLY)) == -1) {
		perror("Client failed to open the MainFIFO\n");
		return 1;
	}
	fprintf(stderr,"---------------------- client :%d -----------------\n",getpid());
	sprintf(clientPid,"%d",getpid());
	sprintf(logFileName,"%s.log",clientPid);
	logOutput=fopen(logFileName,"w");
	/*-----------------------------------------------------------------------------------*/
	sprintf(string,"%s %s %s %s %s",clientPid,&argv[1][1],&argv[2][1],&argv[3][1],&argv[4][1]);
	write(mainFifo,string,strlen(string)+1);
	/*-----------------------------------------------------------------------------------*/
	if ((mkfifo(clientPid,0666) == -1)) {
		perror("2Server failed to create the MainFIFO\n");
		return 1;
	}
	if ((clientFifo = open(clientPid, O_RDONLY)) == -1) {
		perror("Client failed to open the MainFIFO\n");
		return 1;
	}
	/*-----------------------------------------------------------------------------------*/
    while(1) {
        read(clientFifo,newString,256);
        if(doneflag==1) {
            fprintf(stderr,"CTRL-C catched...\n");
            clientCounter=fopen("clientCount.txt","r");
		    fscanf(clientCounter,"%d",&count);
		    fclose(clientCounter);
		    clientCounter=fopen("clientCount.txt","w");
		    fprintf(clientCounter,"%d",count-1);				//clients die and client number decrease 1
		    fprintf(logOutput,"CTRL-C catched\n");
		    fprintf(stderr,"Client is succeesfuly was stopped\n");
		    fprintf(stderr,"----------------------------------------------------------\n");
		    fclose(clientCounter);
		    fclose(logOutput);

		    unlink(clientPid);
            kill(getpid(), SIGKILL);
            return 0;
        }
        fprintf(stderr,"%s / client :%s\n",newString,clientPid); 
        fprintf(logOutput,"%s / client id:%s\n",newString,clientPid);
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////
int argumanCheck(char arr[10]) {
    if(strcmp(arr,"f1")!=0||strcmp(arr,"f2")!=0||strcmp(arr,"f3")!=0||
            strcmp(arr,"f4")!=0||strcmp(arr,"f5")!=0||strcmp(arr,"f6")!=0)
        return 0;
    return 1;
}
/////////////////////////////////////////////////////////////////////////////////////////////
int operandCheck(char arr[10]) {
    if(strcmp(arr,"+")!=0||strcmp(arr,"-")!=0||strcmp(arr,"*")!=0||strcmp(arr,"/")!=0)
        return 0;
    return 1;
}
void intHandler(int status) {
    doneflag = 1;
}
/////////////////////////////////////////////////////////////////////////////////////////////
