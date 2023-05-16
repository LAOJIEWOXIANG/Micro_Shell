#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include "defn.h"

static char* list[] = {"exit", "envset", "envunset", "cd", "shift", "unshift", "sstat"};
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
            r_value = 1;
        }
        exit(exit_value);
    }
}

void exec_envset() {
    char* new_value = command[2];
    if (command[2] == NULL) {
        fprintf(stderr, "envset need a value\n");
        r_value = 1;
        return;
    }
    int ret = setenv(command[1], new_value, 1);
     if (ret != 0) {
        perror("setenv");
        r_value = 1;
        return;
    }
}

void exec_envunset() {
    if (command[2] != NULL) {
        fprintf(stderr, "envunset only takes one argument\n");
        r_value = 1;
        return;
    }
    printf("goes\n");
    if (unsetenv(command[1]) != 0) {
        perror("envunset");
        r_value = 1;
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
        r_value = 1;
        return;
    }
}

void exec_shift() {
    int cur_shift = 0;
    if (command[1] == NULL) {
        cur_shift = 1;
        shift += cur_shift;
    } else {
        cur_shift = atoi(command[1]);
        shift += cur_shift;
    }
    if ((args - shift) < 0) {
        fprintf(stderr, "can't shift that many arguments\n");
        r_value = 1;
    } else {
        args = args - cur_shift;
    }
}

void exec_unshift() {
    if (command[1] != NULL) { //  if we were given the unshift value
        if (atoi(command[1]) > shift || atoi(command[1]) > args) {
            fprintf(stderr, "can't unshift that many arguments\n");
            r_value = 1; // unsuccessful built-in
        }
        args += atoi(command[1]);
        shift -= atoi(command[1]);
    } else {
        args = arg_count;
        shift = 0;
    }
   
}
void exec_sstat(int outfd) {
    char perms[11];
    struct stat st;
    if (command[1] == NULL) {
        fprintf(stderr, "sstat needs a file name\n");
        r_value = 1;
        return;
    }
    // int i = 1;
    for (int i = 1; command[i] != NULL; i++) {
        if (stat(command[i], &st) == 0) {
            dprintf(outfd, "%s ", command[i]); //  print file name

            struct passwd *pwd = getpwuid(st.st_uid);
            if (pwd == NULL) { //  print user name
                dprintf(outfd, "%u ", st.st_uid);
            } else {
                dprintf(outfd, "%s ", pwd->pw_name);
            }

            struct group *grp = getgrgid(st.st_gid); // print group name
            if (grp == NULL) {
                dprintf(outfd, "%u ", st.st_gid);
            } else {
                dprintf(outfd, "%s ", grp->gr_name);
            }
            
            strmode(st.st_mode, perms); //  print permission
            dprintf(outfd, "%s", perms);
            dprintf(outfd, "%lu ", st.st_nlink); //  print number of links"
            dprintf(outfd, "%lu ", st.st_size); //  print size
            dprintf(outfd, "%s", asctime(localtime(&st.st_mtime))); //  print last modified time
            fflush(stdout);
        } else {
            fprintf(stderr, "invalid file name: %s\n", command[i]);
            r_value = 1;
            return ;
        }
        // i++;
    }
    
}

int exec_builtin(char** line, int outfd) {
    funcPtr flist[] = {exec_exit, exec_envset, exec_envunset, exec_cd, exec_shift, exec_unshift, exec_sstat};
    command = line;
    is_builtin = 1;
    for (int i = 0; i < sizeof(list)/sizeof(list[0]); i++) {
        // printf("command[0]: %s, list[i]: %s\n", command[0], list[i]);
        if (strcmp(command[0], list[i]) == 0) {
            if (strcmp(command[0], "sstat") == 0) {
                flist[i](outfd);
            } else {}
                flist[i]();
                if (r_value != 1) { // if we didn't encounter an error
                    r_value = 0;
                }
                free(command);
                command = NULL;
                return is_builtin;
        }
    }
    /* didn't find a builtin command */
    is_builtin = -1;
    return is_builtin;
}
