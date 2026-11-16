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

int execute(char** arglist, int background, const char* command) {

    /* Background execution: a supervisor waits for the actual command. */
    if (background) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            return -1;
        }

        if (pid == 0) {
            execute(arglist, 0, command);
            _exit(0);
        }

        add_background_job(pid, command);
        return 0;
    }

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

        int status;
        if (waitpid(pid, &status, 0) < 0) {
            perror("waitpid");
            return 1;
        }

        if (WIFEXITED(status))
            return WEXITSTATUS(status);

        if (WIFSIGNALED(status))
            return 128 + WTERMSIG(status);

        return 1;
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

    if (pid1 < 0) {
        perror("fork");
        close(fd[0]);
        close(fd[1]);
        return -1;
    }

    pid_t pid2 = fork();

    if (pid2 == 0) {
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);
        close(fd[1]);
        run_command(right);
    }

    if (pid2 < 0)
        perror("fork");

    close(fd[0]);
    close(fd[1]);

    int status1, status2;

    waitpid(pid1, &status1, 0);

    if (pid2 > 0) {
        waitpid(pid2, &status2, 0);

        if (WIFEXITED(status2))
            return WEXITSTATUS(status2);

        if (WIFSIGNALED(status2))
            return 128 + WTERMSIG(status2);
    }

    return 1;
}
