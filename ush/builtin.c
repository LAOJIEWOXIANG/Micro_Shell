#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include "defn.h"

static char* list[] = {"exit", "envset", "envunset", "cd", "shift", "unshift", "sstat file"};
typedef void (*funcPtr) ();
static int is_builtin;
static char** command;
// int shift = 0;

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
    char* new_value = command[2];
    int ret = setenv(command[1], new_value, 1);
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
    } else {
        result = chdir(command[1]);
    }
    if (result != 0) {
        perror("chdir");
        return;
    }
}

void exec_shift() {
    shift = atoi(command[1]);
    args = args - shift;
    if (args < 0) {
        fprintf(stderr, "can't shift that many arguments");
        return;
    }
}

void exec_unshift() {
    if (command[1] != NULL) { //  if we were given the unshift value
        args += atoi(command[1]);
        shift -= atoi(command[1]);
    } else {
        args = arg_count;
        shift = 0;
    }
   
}
void exec_sstat() {
    char* file_name;
    char* u_name;
    char* g_name;
    int n_links;
    int size;
    
}

int exec_builtin(char** line) {
    funcPtr flist[] = {exec_exit, exec_envset, exec_envunset, exec_cd, exec_shift, exec_unshift, exec_sstat};
    command = line;
    is_builtin = -1;
    for (int i = 0; i < sizeof(list)/sizeof(list[0]); i++) {
        if (strcmp(command[0], list[i]) == 0) {
            flist[i]();
            is_builtin = 1;
            free(command);
            command = NULL;
            return is_builtin;
        }
    }
    /* didn't find a builtin command */
    return is_builtin;
}
