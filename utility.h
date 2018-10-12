#ifndef COMMAND_H
#define COMMAND_H
#define SIZE 1024
extern char **environ;
extern int errno;
int cds(char **args);
int clr(char **args);
int dir(char **args);
int environt(char **args);
int echos(char **args);
int quits(char **args);
int helps(char **args);
int paused(char **args);
#endif