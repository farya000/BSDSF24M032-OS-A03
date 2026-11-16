#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>

#define MAX_LEN 512
#define MAXARGS 10
#define ARGLEN 30
#define PROMPT "FCIT> "

typedef struct {
    pid_t pid;
    char command[MAX_LEN];
} BackgroundJob;

char* read_cmd(char* prompt, FILE* fp);
char** tokenize(char* cmdline);
int execute(char** arglist, int background, const char* command);
int handle_builtin(char** arglist);
void store_history(char* cmd);
void show_history(void);
char* get_history(int n);

void add_background_job(pid_t pid, const char* command);
void reap_background_jobs(void);
void show_jobs(void);

#endif
