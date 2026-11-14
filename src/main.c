#include "shell.h"

int main() {
    char* cmdline;
    char** arglist;

    while ((cmdline = read_cmd(PROMPT, stdin)) != NULL) {
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
        cmdline = exec_line;

        if ((arglist = tokenize(cmdline)) != NULL) {
            if (!handle_builtin(arglist))
                execute(arglist);

            for (int i = 0; arglist[i] != NULL; i++)
                free(arglist[i]);
            free(arglist);
        }

        free(cmdline);
    }

    printf("\nShell exited.\n");
    return 0;
}
