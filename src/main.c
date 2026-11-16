#include "shell.h"

static void free_arguments(char** arglist) {
    if (arglist == NULL)
        return;

    for (int i = 0; arglist[i] != NULL; i++)
        free(arglist[i]);

    free(arglist);
}

static void process_command(char* command) {
    while (*command == ' ' || *command == '\t')
        command++;

    if (*command == '\0')
        return;

    char* saved_command = strdup(command);
    if (saved_command == NULL)
        return;

    char** arglist = tokenize(command);

    if (arglist == NULL) {
        free(saved_command);
        return;
    }

    int count = 0;
    while (arglist[count] != NULL)
        count++;

    int background = 0;

    if (count > 0 && strcmp(arglist[count - 1], "&") == 0) {
        background = 1;
        free(arglist[count - 1]);
        arglist[count - 1] = NULL;
    }

    if (arglist[0] != NULL) {
        if (!background && handle_builtin(arglist) == 0)
            execute(arglist, 0, saved_command);
        else if (background)
            execute(arglist, 1, saved_command);
    }

    free_arguments(arglist);
    free(saved_command);
}

int main(void) {
    char* cmdline;

    while (1) {
        reap_background_jobs();

        cmdline = read_cmd(PROMPT, stdin);

        if (cmdline == NULL)
            break;

        char* exec_line = strdup(cmdline);

        if (cmdline[0] == '!' && cmdline[1] != '\0') {
            int n = atoi(cmdline + 1);
            char* old_cmd = get_history(n);

            if (old_cmd == NULL) {
                printf("Invalid history number.\n");
                store_history(cmdline);
                free(exec_line);
                free(cmdline);
                continue;
            }

            free(exec_line);
            exec_line = strdup(old_cmd);
            printf("%s\n", exec_line);
        }

        store_history(cmdline);
        free(cmdline);

        if (exec_line == NULL)
            continue;

        /* Split the line at semicolons and execute commands in order. */
        char* saveptr;
        char* command = strtok_r(exec_line, ";", &saveptr);

        while (command != NULL) {
            process_command(command);
            command = strtok_r(NULL, ";", &saveptr);
        }

        free(exec_line);
    }

    reap_background_jobs();
    printf("\nShell exited.\n");
    return 0;
}
