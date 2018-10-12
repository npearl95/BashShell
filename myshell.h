#ifndef MAIN_H
#define MAIN_H
#include "utility.h"
#define _GNU_SOURCE
#define _XOPEN_SOURCE
#define BUFF_SIZE 1024
#define TOKEN_SIZE 64
#define SYMBOLS " \t\r\n\a"
extern int errno;
int main();
char *getCommandLine();
char **ParseTheCommand(char *commandLine);
int execute(char **args);
int startprocess(char **args);
int detect_symbol(char **args);
int get_pos(char **args);
int redirect(char **args1 , char **args2 , int type);
int piping(char **args1 , char **args2);
#endif