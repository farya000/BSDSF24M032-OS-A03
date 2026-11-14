#include "shell.h"

char* read_cmd(char* prompt, FILE* fp) {
    printf("%s", prompt);
    char* cmdline = (char*) malloc(sizeof(char) * MAX_LEN);
    int c, pos = 0;

    while ((c = getc(fp)) != EOF) {
        if (c == '\n') break;
        cmdline[pos++] = c;
    }

    if (c == EOF && pos == 0) {
        free(cmdline);
        return NULL; // Handle Ctrl+D
    }
    
    cmdline[pos] = '\0';
    return cmdline;
}

char** tokenize(char* cmdline) {
    // Edge case: empty command line
    if (cmdline == NULL || cmdline[0] == '\0' || cmdline[0] == '\n') {
        return NULL;
    }

    char** arglist = (char**)malloc(sizeof(char*) * (MAXARGS + 1));
    for (int i = 0; i < MAXARGS + 1; i++) {
        arglist[i] = (char*)malloc(sizeof(char) * ARGLEN);
        bzero(arglist[i], ARGLEN);
    }

    char* cp = cmdline;
    char* start;
    int len;
    int argnum = 0;

    while (*cp != '\0' && argnum < MAXARGS) {
        while (*cp == ' ' || *cp == '\t') cp++; // Skip leading whitespace
        
        if (*cp == '\0') break; // Line was only whitespace

        start = cp;
        len = 1;
        while (*++cp != '\0' && !(*cp == ' ' || *cp == '\t')) {
            len++;
        }
        strncpy(arglist[argnum], start, len);
        arglist[argnum][len] = '\0';
        argnum++;
    }

    if (argnum == 0) { // No arguments were parsed
        for(int i = 0; i < MAXARGS + 1; i++) free(arglist[i]);
        free(arglist);
        return NULL;
    }

    arglist[argnum] = NULL;
    return arglist;
}

int handle_builtin(char** arglist) {
    if (strcmp(arglist[0], "exit") == 0) {
        printf("Shell exited.\n");
        exit(0);
    }
    else if (strcmp(arglist[0], "cd") == 0) {
        char* dir = arglist[1] ? arglist[1] : getenv("HOME");
        if (chdir(dir) != 0)
            perror("cd");
        return 1;
    }
    else if (strcmp(arglist[0], "help") == 0) {
        printf("Built-in commands:\n");
        printf("exit - Exit the shell\n");
        printf("cd <directory> - Change directory\n");
        printf("help - Display this help message\n");
        printf("jobs - Display job control status\n");
        printf("history - Display command history\n");
        printf("!n - Execute a previous command\n");
        return 1;
    }
    else if (strcmp(arglist[0], "history") == 0) {
        show_history();
        return 1;
    }
    else if (strcmp(arglist[0], "jobs") == 0) {
        printf("Job control not yet implemented.\n");
        return 1;
    }
    return 0;
}

#define HISTORY_SIZE 20

static char* history_list[HISTORY_SIZE];
static int history_count = 0;

void add_history(char* cmd) {
    if (history_count == HISTORY_SIZE) {
        free(history_list[0]);
        for (int i = 1; i < HISTORY_SIZE; i++)
            history_list[i - 1] = history_list[i];
        history_count--;
    }
    history_list[history_count++] = strdup(cmd);
}

void show_history(void) {
    for (int i = 0; i < history_count; i++)
        printf("%d  %s\n", i + 1, history_list[i]);
}

char* get_history(int n) {
    if (n < 1 || n > history_count)
        return NULL;
    return history_list[n - 1];
}
