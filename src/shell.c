
#include "shell.h"
#include <readline/readline.h>
#include <readline/history.h>


static char *command_generator(const char *text, int state) {
    static const char *commands[] = {
        "cd", "exit", "help", "jobs", "history",
        "pwd", "whoami", "date", "ls", "cat",
        "mkdir", "rm", "cp", "mv", "clear", "echo", NULL
    };
    static int index, len;

    if (!state) {
        index = 0;
        len = strlen(text);
    }

    while (commands[index] != NULL) {
        const char *cmd = commands[index++];
        if (strncmp(cmd, text, len) == 0)
            return strdup(cmd);
    }
    return NULL;
}

static char **shell_completion(const char *text, int start, int end) {
    (void)end;
    if (start == 0)
        return rl_completion_matches(text, command_generator);
    return rl_completion_matches(text, rl_filename_completion_function);
}

char* read_cmd(char* prompt, FILE* fp) {
    rl_attempted_completion_function = shell_completion;
    rl_bind_key('\t', rl_complete);
    (void)fp;
    return readline(prompt);
}

char** tokenize(char* cmdline) {
    if (cmdline == NULL || cmdline[0] == '\0')
        return NULL;

    char** arglist = malloc(sizeof(char*) * (MAXARGS + 1));

    for (int i = 0; i < MAXARGS + 1; i++) {
        arglist[i] = calloc(ARGLEN, sizeof(char));
    }

    char* cp = cmdline;
    int argnum = 0;

    while (*cp != '\0' && argnum < MAXARGS) {
        while (*cp == ' ' || *cp == '\t') cp++;
        if (*cp == '\0') break;

        int len = 0;
        while (cp[len] != '\0' && cp[len] != ' ' &&
               cp[len] != '\t' && len < ARGLEN - 1) {
            len++;
        }

        strncpy(arglist[argnum], cp, len);
        argnum++;
        cp += len;
    }

    if (argnum == 0) {
        for (int i = 0; i < MAXARGS + 1; i++)
            free(arglist[i]);
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

static char* command_history[HISTORY_SIZE];
static int history_count = 0;

void store_history(char* cmd) {
    if (history_count == HISTORY_SIZE) {
        free(command_history[0]);
        for (int i = 1; i < HISTORY_SIZE; i++)
            command_history[i - 1] = command_history[i];
        history_count--;
    }

    command_history[history_count++] = strdup(cmd);
    add_history(cmd);
}

void show_history(void) {
    for (int i = 0; i < history_count; i++)
        printf("%d  %s\n", i + 1, command_history[i]);
}

char* get_history(int n) {
    if (n < 1 || n > history_count)
        return NULL;
    return command_history[n - 1];
}
