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
    if (command[1] == NULL) {
        shift = 1;
    } else {
        shift = atoi(command[1]);
    }
    if ((args - shift) < 0) {
        fprintf(stderr, "can't shift that many arguments\n");
        return;
    } else {
        args = args - shift;
    }
}

void exec_unshift() {
    if (command[1] != NULL) { //  if we were given the unshift value
        if (atoi(command[1]) > shift) {
            fprintf(stderr, "can't unshift that many arguments\n");
            return;
        }
        args += atoi(command[1]);
        shift -= atoi(command[1]);
    } else {
        args = arg_count;
        shift = 0;
    }
   
}
void exec_sstat() {
    char perms[11];
    struct stat st;
    for (int i = 1; i < sizeof(command); i++) {
        if (stat(command[i], &st) == 0) {
            printf("%s ", command[i]); //  print file name

            struct passwd *pwd = getpwuid(st.st_uid);
            if (pwd == NULL) { //  print user name
                printf("%u ", st.st_uid);
            } else {
                printf("%s ", pwd->pw_name);
            }

            struct group *grp = getgrgid(st.st_gid); // print group name
            if (grp == NULL) {
                printf("%u ", st.st_gid);
            } else {
                printf("%s ", grp->gr_name);
            }
            
            strmode(st.st_mode, perms); //  print permission
            printf("Permissions of myfile.txt: %s ", perms);

            printf("%lu ", st.st_nlink); //  print number of links"
            printf("%lu ", st.st_size); //  print size
            printf("%s\n", asctime(localtime(&st.st_mtime))); //  print last modified time
        }
    }

}

int exec_builtin(char** line) {
    funcPtr flist[] = {exec_exit, exec_envset, exec_envunset, exec_cd, exec_shift, exec_unshift, exec_sstat};
    command = line;
    is_builtin = -1;
    for (int i = 0; i < sizeof(list)/sizeof(list[0]); i++) {
        if (strcmp(line[0], list[i]) == 0) {
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
