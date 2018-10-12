/*
Chau Nguyen
tug37553@temple.edu
Shell project contained
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "myshell.h"
#include "utility.h"
#include "myshell.h"
#include "utility.h"

extern char **environ;
extern int errno;
//cd command
//If nothing after cds then bring user to Home directory
int cds(char **args){
	//If nothing after cds then bring user to Home directory
	if(args[1] == NULL){
		char *cur = getenv("HOME");
		//Print error if getenv("HOME") is not success
		if(cur == NULL)
			fprintf(stderr, "Can't find home");
		if(chdir(cur) == -1)
			fprintf((stderr), "Can't access to directory");
	}
	else{
		if(chdir(args[1]) == -1)
			fprintf(stderr, "Can't change directory");
	}
	return 1;
}

//clr command
//Using this command to print white space and move the pointer on top
int clr(char **args){
	printf("\033[2J\033[1;1H");
	printf("%s" , "\n");
	return 1;
}

//dir command
//This dir function will do:
//If user just type dir , it call getcwd to get current directory
//Then opendir to get info of all the file
//The read the name of the file from a built in struct call dirent
//If uder type dir directory then it will opendir and get info of all the file
int dir(char **args){
	struct dirent *de; 
	char *cur = (char *)malloc(sizeof(*cur) * SIZE);
	if(args[1] == NULL){
		if(cur == NULL)
			fprintf(stderr, "Cannot find current directory");
		cur = getcwd(cur , sizeof(*cur) * SIZE);
	    DIR *dr = opendir(cur);
	    if (dr == NULL)  
            fprintf(stderr , "Cannot open current directory" );
        while ((de = readdir(dr)) != NULL)
            fprintf(stderr, " %s\n" ,  de->d_name);
        closedir(dr);
        free(cur);
   }
	else{  
        DIR *dr = opendir(args[1]);
        if (dr == NULL)  
            fprintf(stderr , "Cannot file directory" );
        while ((de = readdir(dr)) != NULL)
            fprintf(stderr, " %s\n" , de->d_name);
        closedir(dr);    
    }
    return 1;
}   

//get environment variable
//This function used a built in variable call extern char **environ
int environt(char **args){
	printf("\n");
	for (int i = 0; environ[i]!= NULL; i++)
		printf("%s\n", environ[i]);
	return 1;
}

//echo command
//If noth arg after echo than do nothing, else just print all the agrs
int echos(char **args){
	if(args[1] == NULL)
		return 1;
	for(int i = 1; args[i] != NULL ; i++)
		printf("%s ", args[i]);
	printf("\n");
    return 1;
}

//quit command
//return exit(1) whenever it was called
int quits(char **args){
	exit(1);
	return 1;
}

//help command
int helps(char **args){
	//int num_cmds = sizeof(my_cmds)/sizeof(char *);
	// printf("The following are built in:\n");
  	//	for(int i = 0;  i < 8; i++)
	//printf("  %s\n",my_cmds[i]);
	char *man[] = {"more","readme" , NULL};
	startprocess(man);
	return 1;
}

//pause command
//while loop run until user press enter
int paused(char **args){
	printf("Press Enter to continue");
	while(getchar() != '\n');//do nothing empty statement
	return 1;
}



