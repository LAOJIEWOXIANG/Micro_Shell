#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include "defn.h"

const static char* list[] = {"exit", "envset", "envuset", "cd"};
typedef void (*funcPtr) ();
static int is_builtin;
static char** command;

void exec_exit() {
    if (command[1] == NULL) {
        free(command);
        command = NULL;
        exit(0);
    } else {
        int exit_value = atoi(command[1]);
        free(command);
        command = NULL;
        if (exit_value == 0) {
            fprintf(stderr, "not given a valid exit value");
            return;
        }
        exit(exit_value);
    }
}

void exec_envset() {
    char* old_value = getenv(command[1]);
    char* new_value = command[2];
    int ret = setenv(old_value, new_value, 1);
     if (ret != 0) {
        perror("setenv");
        return;
    }
}

void exec_envunset() {
    if (unsetenv(command[1]) == -1) {
        perror("envunset");
        return;
    }
}

void exec_cd() {
    int result = 0;
    if (command[1] == NULL) {
        result = chdir(getenv("HOME"));
    }
    result = chdir(command[1]);
    if (result != 0) {
        perror("chdir");
        return;
    }
}

int exec_builtin(char** line) {
    funcPtr flist[] = {exec_exit, exec_envset, exec_envunset};
    command = line;
    is_builtin = -1;
    for (int i = 0; i < 4; i++) {
        if (strcmp(command[0], list[i]) == 0) {
            flist[i]();
            is_builtin = 1;
            return is_builtin;
        }
    }
    free(command);
    command = NULL;
    /* didn't find a builtin command */
    return is_builtin;
}
