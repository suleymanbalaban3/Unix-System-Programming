/**
 * @author Suleyman Balaban - 121044014
 * @date 8 March 2016
 * @version 1
 * @description This is find occuarence in file by given string.
 * And  display the line and column number to "grepOutputFile.log" file
 *
 */
#include <stdio.h>//libraries
#include <stdlib.h>
#include <string.h>
/**
 * This function make all processing about program,for one file 
 * return occuarence number and print to output file lines and columns
 *
 * @param fileName pointer of the input file
 * @param target will find in file this string
 * @return int target's occuarence in file 
 */
int targetOccInFile(const char * fileName,const char * target);
//Start Of Main
int main(int argc, char const *argv[])
{
	if(argc!=3) { //if wrong argument entered
		fprintf(stderr,"Usage : %s file target\n",argv[0]);
		return EXIT_FAILURE;
	}
	else//if true display occuarence on terminal
		printf("Occuarence : %d\n",targetOccInFile(argv[1],argv[2]));
	return EXIT_SUCCESS;
}//End Of main
int targetOccInFile(const char * fileName,const char * target) {
	FILE * input,* output;//file pointers
	char foundedTarget[100];//readed string from file
	int  targetSize=strlen(target)+1;//target string's size
	int row=1,column=1,letter=0,occuarence=0;

	input=fopen(fileName,"r");
	output=fopen("grepOutputFile.log","a");
	if(output==NULL) {//if output file couldn't open
		fprintf(stderr,"Output File couldn't open!!\n");
		exit(1);
	}
	if(input==NULL) { //if input file couldn't open
		fprintf(stderr,"File can't open!\n");
		exit(1);
	}
	else {
		fprintf(output,"#%s\n",fileName);
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
	}
	fclose(input);//close the files
	fclose(output);
	return occuarence;//return occuarence in file
}
