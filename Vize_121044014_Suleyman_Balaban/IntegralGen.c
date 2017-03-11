/**
 * @author Suleyman Balaban - 121044014
 * @date 20 March 2016
 * @version vize
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
#include <signal.h>
#include <assert.h>
#include <matheval.h>

#define BUFFER_SIZE 256								//defines
#define FIFO_PERMS (S_IRWXU | S_IWGRP| S_IWOTH)
#define BLKSIZE PIPE_BUF
#define FILE_NAME_SIZE 256
#define DEBUG
#define PI 3.14159265
#define FIFONAME "slymn"							//my main fifo name

static volatile int doneflag = 0;					//global variables
static volatile int donectrlZ = 0;
/**
 * This function CTRL-C handler
 * @param status 
 */
void intHandler(int status);
/**
 * This function CTRL-Z handler
 * @param status 
 */
void sigstop(int p);
/**
 * This function calculate integral
 * @param min 
 * @param max 
 * @param resolution 
 * @param left function
 * @param right function
 * @param operator operand name
 * @param error for exception
 * @return double result of integral
 */
double integral(double min,double max,double resolution,char left[80],char right[80],char operator[5],int * error);
/**
 * This function calculate function using libmatheval library
 * @param function string
 * @param deger x number
 * @return double result of function
 */
double fEval(char function[80],double deger);
/**
 * This function take number to using  nanosleep function
 * @param sleep_time number
 * @return int 
 */
int nanoSleep (double sleep_time);
/**
 * This function read file according to arguments
 * @param f1 to find function string
 * @param f2 to find function string
 * @param f3 to find function string
 * @param f4 to find function string
 * @param f5 to find function string
 * @param f6 to find function string
 */
void fileRead(char f1[80],char f2[80],char f3[80],char f4[80],char f5[80],char f6[80]);
/**
 * This function check which file in arguments
 * @param f1 to find which argument of function string
 * @param f2 to find which argument of function string
 * @param f3 to find which argument of function string
 * @param f4 to find which argument of function string
 * @param f5 to find which argument of function string
 * @param f6 to find which argument of function string
 * @param funcName 
 */
void getFunction(char f1[80],char f2[80],char f3[80],char f4[80],char f5[80],char f6[80],char * funcName);

/////////////////////////////////////////////////////////////////////////////////////////////
int main (int argc, char *argv[]) {
    FILE * clientCounter;										//file pointer of countClients
    FILE * outServer;											//file pointer to print server outputs
	int mainFifo,clientFifo;									//to open mainfifo and clients's fifos
	char string[256];											//to take arguments from clients
	char f1[80],f2[80],f3[80],f4[80],f5[80],f6[80];				//functions includes
	pid_t child;												//fork function's variable
	char *clientPid,*leftFunc,*rightFunc,*operation,*interval;	//to parse string for taken arguments from clients
	double intervalValue;										//to store interval_time
	int count=0;												//count clients to check maxClients
	double maxClient;											//this variable assigned with &argv[2][1]
	double resolution;											//this variable assigned with &argv[1][1]
	struct timeval start, stop;									//calculate start time and stop time
    double secs = 0;											//t0 seconds
    /*------------------------------------------------------------------------------------------------------*/
    clientCounter=fopen("clientCount.txt","w");					//server.log file print 0 for start
    fprintf(clientCounter,"%d",0);
    fclose(clientCounter);

    signal(SIGINT, intHandler);									//signal handling
    signal(SIGTSTP,sigstop);
    /*------------------------------------------------------------------------------------------------------*/
	if(argc!=3||argv[1][0]!='-'||argv[2][0]!='-') {				//arguman control
		fprintf(stderr,"You entered wrong argumans or didn't add '-' to argumans...\n");
		exit(1);
	}
	fileRead(f1,f2,f3,f4,f5,f6);								//read files and store in this strings

	maxClient=atof(&argv[2][1]);								
	resolution=atof(&argv[1][1]);
    
    gettimeofday(&start, NULL);									//time start
    /*------------------------------------------------------------------------------------------------------*/
	if ((mkfifo(FIFONAME,0666) == -1)) {						//create main fifo
		perror("1Server failed to create the MainFIFO\n");
		return 1;
	}
    if ((mainFifo = open(FIFONAME,O_RDWR)) == -1) {				//open main fifo
	    perror("4Client failed to open the MainFIFO\n");
	    return 1;
    }
    fprintf(stderr,"Server waiting...\n");
    /*------------------------------------------------------------------------------------------------------*/
	while(1&&donectrlZ==0&&doneflag==0) {						//work while enter ctrl-c or ctrl-z
		//__________________________________________________			
	    if(doneflag==1||donectrlZ==1) {
		    fprintf(stderr,"CTRL-Z catched...\n");
		    kill(getppid(),SIGSTOP);
		    if( unlink(FIFONAME) == -1 ) {
	            perror("Server failed to remove the MainFIFO\n");
            }
		    return 0;
		}    
		//__________________________________________________
        read(mainFifo,string,256);								//read argument from clients by main fifo
        gettimeofday(&stop, NULL);								//end time connected time
outServer=fopen("server.log","a");
	    child=fork();											
	    if(child<0) {
		    perror("Server failed to fork\n");
		    return 1;
	    }else if(child==0) {									//child process
	    	/*--------------------------------------------------------------------------------------------*/
		    clientPid=strtok(string," ");						//parse strig to know clients arguments
		    leftFunc=strtok(NULL," ");
		    rightFunc=strtok(NULL," ");
		    interval=strtok(NULL," ");
		    operation=strtok(NULL," ");
		    intervalValue=atof(interval);

		    clientCounter=fopen("clientCount.txt","r");			//count and print maxClients
	        fscanf(clientCounter,"%d",&count);
	        fclose(clientCounter);
	        fprintf(stderr,"**********************\n");
	        fprintf(stderr,"client counter :%d\n",count+1);
	        fprintf(stderr,"**********************\n");
	        clientCounter=fopen("clientCount.txt","w");
	        fprintf(clientCounter,"%d",count);
	        fclose(clientCounter);


	        getFunction(f1,f2,f3,f4,f5,f6,leftFunc);			//get left and right operands
	        getFunction(f1,f2,f3,f4,f5,f6,rightFunc);

		    fprintf(stderr,"------------- client id  :%s -------------\n",clientPid);
            fprintf(stderr,"left operand :%s\n",leftFunc);
            fprintf(stderr,"right operand :%s\n",rightFunc);
            fprintf(stderr,"interval :%s\n",interval);
            fprintf(stderr,"operation :%s\n",operation);
            secs = (double)(stop.tv_usec - start.tv_usec) / 1000000 + (double)(stop.tv_sec - start.tv_sec);
			fprintf(stderr,"time taken :%f\n",secs);
			fprintf(stderr,"resolution : %f miliseconds\n",resolution);
            fprintf(stderr,"---------------------------------------------\n");
            /*--------------------------------------------------------------------------------------------*/
            if(count<maxClient) {								//if max client number bigger client number

            	clientCounter=fopen("clientCount.txt","r");		//read clients number and increse
		        fscanf(clientCounter,"%d",&count);
		        fclose(clientCounter);
		        count++;
		        clientCounter=fopen("clientCount.txt","w");		//after<that write again 
		        fprintf(clientCounter,"%d",count);
		        fclose(clientCounter);

		        if ((clientFifo = open(clientPid, O_WRONLY)) == -1) {	//open client's fifo
			        perror("3Client failed to open the MainFIFO\n");
			        return 1;
		        }	
		        int intervalTime=intervalValue;
		        
		        fprintf(outServer,"client id :%s  / connection time :%f\n",clientPid,secs);
		        fclose(outServer);
		        fprintf(stderr,"client id :%s  / connection time :%f\n",clientPid,secs);
		        intervalValue=intervalValue+secs;
		        while(1) {
		        	double result;								//result of integration
		        	int exception=0;							//exception divide by zero
            		char sonuc[256];							//string that write to fifo for clients

		            if(doneflag==1||donectrlZ==1) {
		                kill(atoi(clientPid),SIGKILL);
		                unlink(clientPid);						//if server shut down client fifo clean
	                    fprintf(stderr,"CTRL-C catched...\n");
	                    break;
	                }
	                											//my integral function
	                result=integral(secs,intervalValue,resolution/1000.0,leftFunc,rightFunc,operation,&exception);
	                
	                if(exception==1) {							//if exception 1
	                	sprintf(sonuc,"[ %f -> %f ] f(x)= %s %s %s = NO RESULT BECAUSE OF DIVIDE BY ZERO!",
	                		secs,intervalValue,leftFunc,operation,rightFunc);
	                }
	                else
	                	sprintf(sonuc,"[ %f -> %f ] f(x) = %s %s %s = %f",secs,intervalValue,leftFunc,operation,rightFunc,result);

	                intervalValue=intervalValue+intervalTime;	
	                secs=secs+intervalTime;
		            write(clientFifo,sonuc,strlen(sonuc)+1);	//write to client's fifo
		            nanoSleep(intervalTime);

		            if(doneflag==1) {
		                kill(atoi(clientPid),SIGKILL);
		                unlink(clientPid);
	                    fprintf(stderr,"\nCTRL-C catched...\n");
	                    break;
	                }else if(donectrlZ==1) {
		                kill(atoi(clientPid),SIGKILL);
		                unlink(clientPid);
	                    fprintf(stderr,"\nCTRL-Z catched...\n");
	                    break;
	                }
		        }
		    }else {												//max client warning
		    	fprintf(stderr,"/////////////////////////////////////////////////\n");
		        fprintf(stderr,"////////!!!!!!!!!! MAXIMUM CLIENT !!!!!!!!///////\n");
		        fprintf(stderr,"/////!!! You have to wait for new clients!!!/////\n");
		        fprintf(stderr,"/////////////////////////////////////////////////\n");
		        exit(1);
		    }			
		    if(doneflag==1||donectrlZ==1) {
		    	fprintf(stderr,"Data saved...\n");
                fprintf(stderr,"Terminated successfully ended...\n");
		        kill(getppid(),SIGSTOP);//SIGSTOP
		        if( unlink(FIFONAME) == -1 ) {
	                perror("Server failed to remove the MainFIFO\n");
                }
		        break;
		    }
		    exit(1);
	    }   
	}	
	/*fprintf(stderr,"Data saved...\n");
    fprintf(stderr,"Terminated successfully ended...\n");*/
	return EXIT_SUCCESS;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void intHandler(int status) {
    doneflag = 1;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void sigstop(int p){
    donectrlZ=1;
}
/////////////////////////////////////////////////////////////////////////////////////////////
double integral(double min,double max,double resolution,char left[80],char right[80],char operator[5],int * error) {
	double hilk=min,hson=max;
	double h=hson-hilk;
	double n=h/resolution;
	double xi;													//calculate integral using tarnspoze method 
	double toplam=0.0;
	double sonuc;
	double divideControl;
	int i;
	double a;

	for(xi=hilk;xi<=hson;xi+=(h/n)){
		if(strcmp(operator,"+")==0) {							// '+' opearator
			if(xi==hilk||xi==hson)
				toplam+=fEval(left,xi)+fEval(right,xi);
			else
				toplam+=2*(fEval(left,xi)+fEval(right,xi));
		}else if(strcmp(operator,"-")==0) {						// '-' operator
			if(xi==hilk||xi==hson)
				toplam+=fEval(left,xi)-fEval(right,xi);
			else
				toplam+=2*(fEval(left,xi)-fEval(right,xi));
		}
		else if(strcmp(operator,"*")==0) {						// '*' operator
			if(xi==hilk||xi==hson)
				toplam+=fEval(left,xi)*fEval(right,xi);
			else
				toplam+=2*(fEval(left,xi)*fEval(right,xi));
		}
		else if(strcmp(operator,"/")==0) {						// '/' operator 
			divideControl=fEval(right,xi);
			if(divideControl==0) {								//exception
				*error=1;
				return -99999;
			}
			if(xi==hilk||xi==hson)
				toplam+=fEval(left,xi)/fEval(right,xi);
			else
				toplam+=2*(fEval(left,xi)/fEval(right,xi));
		}
	}
	sonuc=(toplam*(h/n))/2;
	return sonuc;												//return integral result
}
/////////////////////////////////////////////////////////////////////////////////////////////
double fEval(char function[80],double deger) {
	int length;			
	void *f;													//libmatheval library to parse and calculate equation
	int i;	
	double sonuc;	

	length = strlen (function);
	if (length > 0 && function[length - 1] == '\n')
		function[length - 1] = '\0';

	f = evaluator_create (function);
	assert (f);

	sonuc=evaluator_evaluate_x (f, deger);
	evaluator_destroy (f);

	return sonuc;
}
/////////////////////////////////////////////////////////////////////////////////////////////
int nanoSleep (double sleep_time) {
	struct timespec tv;
	tv.tv_sec = (time_t) sleep_time;
	tv.tv_nsec = (long) ((sleep_time - tv.tv_sec) * 1e+9);

	while (1) {
		int rval = nanosleep (&tv, &tv);						//my own nanosleep function 
		if (rval == 0)
			return 0;
		else if (errno == EINTR)
			continue;
		else 
			return rval;
	}
	return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void fileRead(char f1[80],char f2[80],char f3[80],char f4[80],char f5[80],char f6[80]) {
	FILE * input;										

	input=fopen("f1.txt","r");									//read functions from files
	fgets(f1,80,input);
	fclose(input);

	input=fopen("f2.txt","r");
	fgets(f2,80,input);
	fclose(input);

	input=fopen("f3.txt","r");
	fgets(f3,80,input);
	fclose(input);

	input=fopen("f4.txt","r");
	fgets(f4,80,input);
	fclose(input);

	input=fopen("f5.txt","r");
	fgets(f5,80,input);
	fclose(input);

	input=fopen("f6.txt","r");
	fgets(f6,80,input);
	fclose(input);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void getFunction(char f1[80],char f2[80],char f3[80],char f4[80],char f5[80],char f6[80],char * funcName) {
	if(strcmp(funcName,"f1")==0)					
		strcpy(funcName,f1);									//find which function is online
	else if(strcmp(funcName,"f2")==0)
		strcpy(funcName,f2);
	else if(strcmp(funcName,"f3")==0)
		strcpy(funcName,f3);
	else if(strcmp(funcName,"f4")==0)
		strcpy(funcName,f4);
	else if(strcmp(funcName,"f5")==0)
		strcpy(funcName,f5);
	else if(strcmp(funcName,"f6")==0)
		strcpy(funcName,f6);
}
/////////////////////////////////////////////////////////////////////////////////////////////