#include "shell.h"

static void run_command(char** tokens) {
    char* args[MAXARGS + 1];
    int count = 0;
    char* input_file = NULL;
    char* output_file = NULL;

    for (int i = 0; tokens[i] != NULL; i++) {
        if (strcmp(tokens[i], "<") == 0) {
            if (tokens[i + 1] == NULL) {
                fprintf(stderr, "Missing input filename\n");
                exit(1);
            }
            input_file = tokens[++i];
        }
        else if (strcmp(tokens[i], ">") == 0) {
            if (tokens[i + 1] == NULL) {
                fprintf(stderr, "Missing output filename\n");
                exit(1);
            }
            output_file = tokens[++i];
        }
        else if (count < MAXARGS) {
            args[count++] = tokens[i];
        }
    }

    args[count] = NULL;

    if (count == 0)
        exit(0);

    if (input_file) {
        int fd = open(input_file, O_RDONLY);
        if (fd < 0) {
            perror(input_file);
            exit(1);
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }

    if (output_file) {
        int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) {
            perror(output_file);
            exit(1);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }

    execvp(args[0], args);
    perror(args[0]);
    exit(1);
}

int execute(char** arglist) {
    int pipe_pos = -1;

    for (int i = 0; arglist[i] != NULL; i++) {
        if (strcmp(arglist[i], "|") == 0) {
            pipe_pos = i;
            break;
        }
    }

    if (pipe_pos == -1) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            return -1;
        }

        if (pid == 0)
            run_command(arglist);

        waitpid(pid, NULL, 0);
        return 0;
    }

    arglist[pipe_pos] = NULL;
    char** left = arglist;
    char** right = &arglist[pipe_pos + 1];

    if (left[0] == NULL || right[0] == NULL) {
        fprintf(stderr, "Invalid pipe command\n");
        return -1;
    }

    int fd[2];
    if (pipe(fd) < 0) {
        perror("pipe");
        return -1;
    }

    pid_t pid1 = fork();
    if (pid1 == 0) {
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]);
        close(fd[1]);
        run_command(left);
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);
        close(fd[1]);
        run_command(right);
    }

    close(fd[0]);
    close(fd[1]);

    if (pid1 > 0) waitpid(pid1, NULL, 0);
    if (pid2 > 0) waitpid(pid2, NULL, 0);

    return 0;
}
