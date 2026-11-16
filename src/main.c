#include "shell.h"

#define MAX_IF_LINES 100

static void free_arguments(char** arglist) {
    if (arglist == NULL)
        return;

    for (int i = 0; arglist[i] != NULL; i++)
        free(arglist[i]);

    free(arglist);
}

static char* trim(char* s) {
    while (*s == ' ' || *s == '\t')
        s++;

    size_t len = strlen(s);
    while (len > 0 &&
           (s[len - 1] == ' ' || s[len - 1] == '\t' ||
            s[len - 1] == '\n'))
        s[--len] = '\0';

    return s;
}

static int process_command(char* command) {
    command = trim(command);

    if (*command == '\0')
        return 0;

    char* saved_command = strdup(command);
    if (saved_command == NULL)
        return 1;

    char** arglist = tokenize(command);

    if (arglist == NULL) {
        free(saved_command);
        return 0;
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

    int result = 0;

    if (arglist[0] != NULL) {
        if (background) {
            result = execute(arglist, 1, saved_command);
        } else if (handle_builtin(arglist) == 0) {
            result = execute(arglist, 0, saved_command);
        }
    }

    free_arguments(arglist);
    free(saved_command);

    return result;
}

static int process_line(char* line) {
    char* saveptr;
    char* command = strtok_r(line, ";", &saveptr);
    int status = 0;

    while (command != NULL) {
        status = process_command(command);
        command = strtok_r(NULL, ";", &saveptr);
    }

    return status;
}

static int is_keyword(char* line, const char* keyword) {
    return strcmp(trim(line), keyword) == 0;
}

static int is_if_line(char* line) {
    char* p = trim(line);

    return p[0] == 'i' && p[1] == 'f' &&
           (p[2] == '\0' || p[2] == ' ' || p[2] == '\t');
}

static void free_lines(char** lines, int count) {
    for (int i = 0; i < count; i++)
        free(lines[i]);
}

static int execute_if(char* first_line) {
    char* condition = trim(first_line) + 2;
    condition = trim(condition);

    if (*condition == '\0') {
        condition = read_cmd("if> ", stdin);
        if (condition == NULL)
            return 1;

        condition = strdup(condition);
        if (condition == NULL)
            return 1;
    } else {
        condition = strdup(condition);
        if (condition == NULL)
            return 1;
    }

    char* line;
    int found_then = 0;

    while ((line = read_cmd("if> ", stdin)) != NULL) {
        if (is_keyword(line, "then")) {
            found_then = 1;
            free(line);
            break;
        }

        printf("Expected 'then'.\n");
        free(line);
        free(condition);
        return 1;
    }

    if (!found_then) {
        free(condition);
        return 1;
    }

    char* then_lines[MAX_IF_LINES];
    char* else_lines[MAX_IF_LINES];
    int then_count = 0;
    int else_count = 0;
    int in_else = 0;
    int found_fi = 0;

    while ((line = read_cmd("if> ", stdin)) != NULL) {
        if (is_keyword(line, "fi")) {
            found_fi = 1;
            free(line);
            break;
        }

        if (is_keyword(line, "else")) {
            in_else = 1;
            free(line);
            continue;
        }

        if (!in_else && then_count < MAX_IF_LINES)
            then_lines[then_count++] = strdup(line);
        else if (in_else && else_count < MAX_IF_LINES)
            else_lines[else_count++] = strdup(line);

        free(line);
    }

    if (!found_fi) {
        printf("Error: missing 'fi'.\n");
        free(condition);
        free_lines(then_lines, then_count);
        free_lines(else_lines, else_count);
        return 1;
    }

    int status = process_line(condition);
    free(condition);

    if (status == 0) {
        for (int i = 0; i < then_count; i++)
            process_line(then_lines[i]);
    } else {
        for (int i = 0; i < else_count; i++)
            process_line(else_lines[i]);
    }

    free_lines(then_lines, then_count);
    free_lines(else_lines, else_count);

    return 0;
}

int main(void) {
    while (1) {
        reap_background_jobs();

        char* cmdline = read_cmd(PROMPT, stdin);

        if (cmdline == NULL)
            break;

        if (cmdline[0] == '!' && cmdline[1] != '\0') {
            int n = atoi(cmdline + 1);
            char* old_cmd = get_history(n);

            if (old_cmd == NULL) {
                printf("Invalid history number.\n");
                store_history(cmdline);
                free(cmdline);
                continue;
            }

            char* exec_line = strdup(old_cmd);
            printf("%s\n", exec_line);
            store_history(cmdline);
            free(cmdline);
            free(cmdline = NULL);

            if (exec_line != NULL) {
                if (is_if_line(exec_line))
                    execute_if(exec_line);
                else
                    process_line(exec_line);

                free(exec_line);
            }

            continue;
        }

        store_history(cmdline);

        if (is_if_line(cmdline))
            execute_if(cmdline);
        else
            process_line(cmdline);

        free(cmdline);
    }

    reap_background_jobs();
    printf("\nShell exited.\n");
    return 0;
}
