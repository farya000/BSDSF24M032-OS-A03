
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

    char** args = calloc(MAXARGS + 1, sizeof(char*));
    int count = 0;
    char* p = cmdline;

    while (*p) {
        while (*p == ' ' || *p == '\t')
            p++;
        if (*p == '\0')
            break;

        if (*p == '<' || *p == '>' || *p == '|') {
            if (count >= MAXARGS) break;
            args[count++] = strndup(p, 1);
            p++;
            continue;
        }

        char token[ARGLEN * 8];
        int len = 0;
        char quote = '\0';

        while (*p) {
            if (quote) {
                if (*p == quote) {
                    quote = '\0';
                    p++;
                } else if (len < (int)sizeof(token) - 1) {
                    token[len++] = *p++;
                } else {
                    p++;
                }
            } else if (*p == '"' || *p == '\'') {
                quote = *p++;
            } else if (*p == ' ' || *p == '\t' ||
                       *p == '<' || *p == '>' || *p == '|') {
                break;
            } else if (len < (int)sizeof(token) - 1) {
                token[len++] = *p++;
            } else {
                p++;
            }
        }

        token[len] = '\0';
        if (len > 0 && count < MAXARGS)
            args[count++] = strdup(token);
    }

    args[count] = NULL;

    if (count == 0) {
        free(args);
        return NULL;
    }

    return args;
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
