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
extern char **environ;
extern int errno;
//This array contains the 8 commands that user can type
//This is be a triger to call the 8 built commands
char *my_cmds[] = {"cds","clr","dir","environs","echos","helps","pause","quit"};

/*my_funcs array will be call whenever user input matched with one of the
8 commands in my_cmds*/
int (*my_funcs[]) (char **) = {&cds,&clr,&dir,&environt,&echos,&helps,&paused,&quits};

//Main function contain while loop run until the execute return error
//This will get get current directory, read user input,
//break user input into separate args, call Execute funtion and return 
//Free malloc
int main(){
	char **args;
	int running;
	printf("Welcome to my shell\n");
	printf("List of command below:\n");
	printf("	cds ,clr, dir, environs\n");
	printf("	echos, helps, pause, quit\n"); 
	printf("	> < and ^(append on file)\n");
	do{
		//get current directory and print prompt
		char *prompt;
		prompt = getcwd(prompt , BUFF_SIZE);
		printf("[%s #: " , prompt);
		//read user input
		char *input;
		input = getCommandLine();
		//break user input into separate args
        args = ParseTheCommand(input);
		//call Execute funtion
		//and return 1 if it's still running
        running = execute(args);
		//Free malloc
		free(input);
        free(args);
	}while(running);

}


/*Creat process by forking
First it will flag background to be 0
If user enter & at the command then set bg to true
Fork to create child process
The child will call execute the command
if bg is true then do thing
if bg is false then make parent wait by calling waitpid(pid , &status , WUNTRACED);
*/
int startprocess(char **args){
	pid_t pid;
	int status = 0;
	int bg = 0;//Background is false

	//If last char is &, the parent DOES NOT HAVE TO WAIT
	if(args[0][strlen(args[0])-1] == '&'){
		bg = 1;//Background is true
		args[0][strlen(args[0])-1] = '\0';
	}
        // if no & just copy args[0] into proc_name
       // strcpy(proc_name, args[0]);
    //}
	//Start forking
	pid = fork();
	//Fork unsuccess
	if(pid < 0){
		printf("%s\n", "Forking child process failed");
		exit(1);
	//child
	}else if(pid == 0){
		//execute the command
		//The created child does not have to run the same program the parent does
		//The execvp() allow process to run any program files
		//execvp(string contain name of a file to be excecuted, pointer to array )
		execvp(args[0] , args);
		exit(errno);
	//parent
	}else{
		//if background process, wait
		if(bg == 0){
			do{
				//pid: 
				//< -1 wait for any child process have some value of pid
				//-1: wait for the child to process
				//0: wait for any child process ID = call process
				//>0: wait for child process ID > call process
				//In this case is pid >0
				//WUNTRACED return if a child had stopped 
				waitpid(pid , &status , WUNTRACED);
			}while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }
	}
	return 1;
}
/*Get the special symbol and position that the symbol located
If it's not < > | and if arg match to 8 internal commands
Then call the internal funcs and create the process
Else it < > |
get arg1 [special character] arg1
call 4 case with piping and redirect function
*/

int execute(char **args){
	//if no arg return 1
	if(args[0] == NULL)
		return 1;
	//Get the symbol
	int symbol = detect_symbol(args);
	int position = get_pos(args);
	//printf("symbol %d\n", symbol);
	//printf("position %d\n", position);	
	//If it's not < > | and if arg match to 8 internal commands
	//Then call the internal funcs and create the process
	//Get the size of the array, number of commands
	int lencmds = sizeof(my_cmds)/sizeof(char *);
	if(symbol <0 || position <0){
		for(int i = 0;  i < lencmds; i++){
			if(strcmp(args[0] , my_cmds[i]) == 0)
				return (*my_funcs[i])(args);
		}
		//Create process by forking 
		return startprocess(args);
	
	//Else it < > |
	}
	//printf("command not found");

	//Get the number of the args
	int len = 0;
	for(int i = 0 ; args[i] != NULL ; i++){
		len++;
	}
		//creat 2 args: arg1 symbol arg2
        char *args1[position + 1];
        char *args2[len - position];
		//Parse the arg intp arg1 and arg2
		 int i = 0;
        for (; i < position; i++)
            args1[i] = args[i];
        args1[i++] = NULL;

        int k = 0;
        for (int j = position + 1; args[j] != NULL; k++, j++)
            args2[k] = args[j];
        args2[k++] = NULL;

	//Now we have 3 cases
	// | < >
	//Call 3 case with piping and redirect function
	//0 is pipe
    if(symbol == 0)
		return piping(args1 , args2);
	//1 is >
	else if(symbol == 1)
		return redirect(args1 , args2, 1);
	//2 is <
	else if(symbol == 2)
		return redirect(args1 , args2, 2);
	//3 is >> (^)
	else if(symbol == 3)
		return redirect(args1 , args2, 3);
	return 1;
}
//IO REDIRECT
/*This will fork and creat a child. This child will run one
of 3 cases > < >>
*/
int redirect(char **args1 , char **args2 , int type){
	
	int fd[2]; //file descriptor
	pid_t pid;
	char *file;
	file = args2[0];
	int status = 0;
	//start forking
	pid = fork();
	if(pid < 0){
		printf("%s\n", "Forking failed");
		exit(1);
	}else if(pid == 0){
	//Child
	//input redirection
	//arg1 < arg2
		if(type == 2){ 
			//open file to read
			//open is a system call
			//O_RDONLY is read only
			fd[0] = open(file, O_RDONLY , 0755); 
			//open() return negative number if it's error
			if(fd[0] <0){
				printf("Error % d\n", errno);
				perror("Error opening file descriptor to read");
				return EXIT_FAILURE;
			}
			//duplicate file descriptoe with stdin
			dup2(fd[0] , STDIN_FILENO);
			//close fd 
			close(fd[0]); 
		}
		//output redirection
		//arg1 > arg2
		else if(type == 1){ 
			//open file to write
			//O_WRONLY is write only
			//O_CREAT if the file doesn't exit, create it, clear all data
			fd[1] = open(file , O_WRONLY | O_CREAT | O_TRUNC, 0755); 
			if(fd[1] < 0){
				printf("Error %d\n", errno);
				perror("Error opening file descriptor to write");
				return EXIT_FAILURE;
			}
			//duplicate file descriptor with stdout
			dup2(fd[1] , STDOUT_FILENO); 
			//close fd
			close(fd[1]); 
		}

		//output redirection
		//append file
		//arg1 >> arg2
		else if(type == 3){ 
			//open file to write
			//O_WRONLY is write only
			//O_CREAT if the file doesn't exit, create it, clear all data
			fd[1] = open(file , O_WRONLY | O_APPEND | O_CREAT , 0755); 
			if(fd[1] < 0){
				printf("Error %d\n", errno);
				perror("Error opening file descriptor to write");
				return EXIT_FAILURE;
			}
			//duplicate file descriptor with stdout
			dup2(fd[1] , STDOUT_FILENO); 
			//close fd
			close(fd[1]); 
		}
		//execute the argument
		execvp(args1[0] , args1); 
		exit(errno);
	}
	else{
		//pid >0
		waitpid(pid , &status , WUNTRACED);
	}
	
	return 1;
}

//piping two commnads
/*
fork first time: child duplicate file descriptor to write from stdout
close the pipe and execute the arg1
In the parent, fork again, the child in the parent duplicate fd to read from stdin
close the pipe and excute the arg2
In the parent parent, close 2 sides of the pipe
*/
int piping(char **args1 , char **args2){
	pid_t pid1 , pid2;
	int s1 , s2 = 0;
    int fd[2];
    if(pipe(fd) == -1){ //create pipe for file descriptor
        printf("Error %d\n", errno);
        perror("Fail creating pipe");
    }
    pid1 = fork();
	//Fork first time 
    if(pid1 < 0){
    	printf("%s\n", "Forking failed"); //fail to create
		exit(1);
    }
    else if(pid1 == 0){
		//child
		//close read
    	close(fd[0]); 
        dup2(fd[1] , STDOUT_FILENO); // duplicate file descriptor to write from stdout
        close(fd[1]);//close file descriptor
        execvp(args1[0] , args1); //execute args1
        exit(errno);
    }
    else{
		//parent fork again
    	pid2 = fork();
    	if(pid2 < 0){
    		printf("%s\n", "Forking child process failed"); //fail to create
			exit(1);
    	}
    	else if(pid2 == 0){
    		close(fd[1]);//close unncessary part(write)
			//duplicate fd to read from stdin
    		dup2(fd[0] , STDIN_FILENO); 
			//close file descriptor
    		close(fd[0]);
			//execute args2
			execvp(args2[0] , args2); 
        	exit(errno);
    	}
    	else{
			//parent don't do anything so close both side of the pipe
    		close(fd[0]);
			//close file descriptors
            close(fd[1]); 
			//wait
            waitpid(pid1 , &s1 , WUNTRACED);
            waitpid(pid2 , &s2 , WUNTRACED); 
    	}
    }
    return 1;
}
//This fuction will read user input everything until EOF ot \n
char *getCommandLine(){
	int ch;
	int buff_size = BUFF_SIZE;
	int i = 0;
	char *buff = malloc(sizeof(char)*buff_size);
	
	if(buff == NULL)
		printf("Malloc Error");
	while(1){	
		ch = getchar();
		if(ch == EOF || ch == '\n'){
			buff[i] = '\0';
			return buff;
		}
		else
			buff[i] = ch;
		i++;
		if(i > buff_size){
			buff_size += BUFF_SIZE;
			buff = realloc(buff , sizeof(char)*buff_size);
			if(buff == NULL){
				printf("Reallocate Error");
			}
		}
	}
	free(buff);
}

//This function will get the whole line and break into pieces
char **ParseTheCommand(char *commandLine){
	int i = 0;
	char **tokenArray;
    char *token;
    if ((tokenArray = malloc(sizeof(char*) * TOKEN_SIZE)) == NULL)
        exit(EXIT_FAILURE);
    token = strtok(commandLine, SYMBOLS);
    while (token != NULL) {
        tokenArray[i++] = token;
        token = strtok(NULL, SYMBOLS);
    }
    tokenArray[i] = NULL;
    return tokenArray;

}
//Find out the | < > or other characters
//This will return 0 for pip, 1 for output redirection and 2 for input redirection
int detect_symbol(char **args){
	if(args[0] == NULL)
		return -1;
	for(int i=0; args[i] != NULL; i++){
		if(args[i][0] == '|') return 0;
		else if(args[i][0] == '>') return 1; //output redirection
		else if(args[i][0] == '<') return 2; //input redirection
		else if(args[i][0] == '^') return 3;//append
	}
	return -1;
}
//Get the position of the special characters
int get_pos(char **args){
	if(args[0] == NULL)
		return -1;
	//Get and return position of | > <
	for(int i = 0; args[i] != NULL; i++){
		if(args[i][0] == '|' || args[i][0] == '>'
			|| args[i][0] == '<'|| args[i][0] == '^')
			return i;
	}
	return -1;
}