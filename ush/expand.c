#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include "defn.h"

void cat(char* new, char* to_cat, int newsize) {
    if (strlen(to_cat) + strlen(new) <= newsize - 1) {
        strcat(new, to_cat);
    } else {
        fprintf(stderr, "No enough space to add");
    }
}

int expand (char *orig, char *new, int newsize) {
    // need a pointer points to the first char of NAME
    char *name = orig;
    // another pointer finds the first '}' and set it to '\0'
    char *end = orig;
    int result = 0;
    int index = 0;
    char* value = 0;
    char append = 0;
    char pid_str[16] = {0};

    while (*name != '\0' && *end != '\0') {
        while (*name != '{') {
            if (*name == '\0') { //  if we never read a {
                return result;
            }
            if (*name != '$') {
                index = name - orig;
                append = orig[index];
                cat(new, &append, newsize);
            } else if (*name == '$'){
                if (*(++name) == '$') {
                    if (sprintf(pid_str, "%d", getpid()) >= 0) {
                        cat(new, pid_str, newsize);
                    } else {
                        fprintf(stderr, "failed to get pid");
                    }
                } else if (*name != '{') {
                    name--;
                    cat(new, name, newsize);
                    return result;
                } else if (*name == '{') {
                    break;
                }
            }
            name++;
        }
        name++;
        //set the last char of orig to '\0', now name points to a string
        while (*end != '}') {
            if (*end == '\0') {
                fprintf(stderr, "Error: missing '}'\n");
                result = -1;
                return result;
            }
            end++;
        }
        *end = '\0';
        value = getenv(name);
        if (value == NULL) {
            cat(new, "", newsize);
        } else {
            cat(new, value, newsize);
        }
        *end = '}'; // set it back to '}
        end++;
        name = end;
    }
    result = 1;
    // printf("new is %s\n", new);
    return result;
}
