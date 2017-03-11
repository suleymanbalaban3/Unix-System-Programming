/**
 * @author Suleyman Balaban - 121044014
 * @date 15 March 2016
 * @version 2
 * @link http://www.thegeekstuff.com/2012/03/c-process-control-functions/
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
//global variable for sum of occuarence in al files
int allOccuarence=0;

#define SIZE 256 //maksimum dosya ismi boyutu

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
int grepFromFile(FILE *inputFile,char *fileName,char *target,FILE *outputFile);
/**
 * This function make process,call grepFromFile function for each files 
 * and print sum of occuarence for all files in outerForRes
 * @param directoryName directory name
 * @param target will find in file this string
 * @param outputFile file pointer for print the results to file
 * @param flag for malloc it will increase for when each file read
 * @param counterFile file pointer for print occuarences
 */
void grepFromDirectory(char *directoryName,char *target,FILE *outputFile,int flag,FILE *counterFile);
/**
 * This function check is directory or .txt 
 * @param directoryName directory name
 * @return BOOL if txt: TRUE,if directory :FALSE return 
 */
BOOL isTxt(char *directoryName);
//Start Of Main
int main(int argc,char *argv[]) {
    FILE *outputFile,*counterFile;//file pointer gfd.log,outerForRes
    int control,result=0,occ=0;//for EOF control,all occ,one file's occ
    int flag=0;//for using recursive grepfromDirectory function  
    //open and send to after checked function 
    if((counterFile=fopen("outerForRes.txt","w+"))==NULL) {
        printf("Dosya Hatasi : %s\n",strerror(errno));
        exit(0);
    }
    if(argc!=3) {//arguman check
        fprintf(stderr,"Usage : %s [directory] [target]\n",argv[0]);
        exit(0);
    }//open and send to after checked function 
    if((outputFile=fopen("gfd.log","a"))==NULL) {
        printf("Dosya Hatasi : %s\n",strerror(errno));
        exit(0);
    }   
    //my most important function that call other function in it     
    grepFromDirectory(argv[1],argv[2],outputFile,flag,counterFile);

    fclose(outputFile);//close file
    fclose(counterFile);//close file
    //open counterFile to read occurrences for each file
    if((counterFile=fopen("outerForRes.txt","r"))==NULL) {
        printf("Dosya Hatasi : %s\n",strerror(errno));
        exit(0);
    }
    while(control=fscanf(counterFile,"%d",&occ)!=EOF) {
        result+=occ;//read and sum occurrences into global variables
    }
    printf("Sum Of Occurrence For All Files : %d\n",result );
    fclose(counterFile);//close file
    return 0;
}//End Of Main
/*##################################################################################################*/
void grepFromDirectory(char *directoryName,char *target,FILE *outputFile,int flag,FILE *counterFile) {  
    DIR *dirPointer;//directory pointer
    struct dirent *dirInfo;
    FILE *inputFile;
    int occuarence=0;
    char *fileName;
    pid_t pid;

    dirPointer=opendir(directoryName);//open directory
    if (dirPointer==NULL) {
        perror("open directory");
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
                   allOccuarence+=grepFromFile(inputFile,fileName,target,outputFile);
                   fprintf(counterFile,"%d ",allOccuarence);//print each file's occurrence 
                   exit(0);
                }
                else
                   wait(NULL);
            }
            else//if directory : call self so,recursive
                grepFromDirectory(fileName,target,outputFile,flag,counterFile);         
            free(fileName);//free dinamic filenName string
        }
    }
    closedir(dirPointer);//close directory
}
/*###############################################################################*/
int grepFromFile(FILE *inputFile,char *fileName,char *target,FILE *outputFile) {
    int targetSize=strlen(target)+1;
    int letter=0;
    int row=1,column=1;
    int occuarence=0;
    char * foundedTarget=(char*)malloc(strlen(target)*sizeof(char));
    if((inputFile=fopen(fileName,"r"))==NULL) {//if input file couldn't open
        printf("Dosya Hatasi : %s\n",strerror(errno));
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
    if(occuarence==0)
        fprintf(outputFile,"  !!!There is no \"%s\" in this file\n",target);
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
/*********************** END OF HW02_121044014_Suleyman_Balaban.c *********************/