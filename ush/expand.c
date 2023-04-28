#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include "defn.h"

void cat(char* new, char* to_cat, int* space) {
    if (strlen(to_cat) + strlen(new) <= *space) {
        strcat(new, to_cat);
        *space -= strlen(new);
    } else {
        fprintf(stderr, "No enough space to add");
    }
}

int expand (char *orig, char *new, int newsize) {
    // need a pointer points to the first char of NAME
    char *name = orig;
    // another pointer finds the first '}' and set it to '\0'
    char *end = orig;
    int result = 0; //  result of expand
    char* value = 0; // the value of the environment variable
    
    char pid_str[16] = {0};
    int space = newsize;
    bool has_quote = false; //  if we read a ${, we set it to true

    while (*name != '\0' && *end != '\0') {
        while (*name != '{') {
            if (*name == '\0') { //  if we never read a {
                return result;
            }
            if (*name != '$') {
                char append[1] = {0};
                append[0] = orig[name - orig];
                append[1] = '\0';
                cat(new, append, &space);
            } else if (*name == '$'){
                name++;
                if (*name == '$') { //  this will increment name
                    if (sprintf(pid_str, "%d", getpid()) >= 0) {
                        cat(new, pid_str, &space);
                    } else {
                        fprintf(stderr, "failed to get pid");
                    }
                } else if (*name == '{') {
                    has_quote = !has_quote;
                    break;
                } else if (isdigit(*name)) {
                    char num[10] = {0};
                    if (args > 0) {
                        while (isdigit(*name)) {
                            char n = *name;
                            strcat(num, &n);
                            name++;
                        }
                        int index = atoi(num);
                        if (index > args) {
                            cat(new, "", &space);
                        } else {
                            cat(new, command_line[index + 1], &space); //  out of bounds?
                        }
                    } else {
                        if (atoi(num) == 0) {
                            cat(new, getenv("SHELL"), &space);
                        } else {
                            cat(new, "", &space);
                        }
                    }
                } else if (*name == '#') {
                    printf("here\n");
                    char pound[3] = {0};
                    if (sprintf(pound, "%d", (args - 1)) >= 0) {
                        printf("%s\n", pound);
                        cat(new, pound, &space);
                    } else {
                        fprintf(stderr, "failed to get #");
                    }
                }else { //  if we read a $ that is not a ${ or $$, we do nothing
                    name--;
                    cat(new, name, &space);
                    return result;
                }
            }
            name++;
        }
        name++;
        //set the last char of orig to '\0', now name points to a string
        if (has_quote == true) {
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
                cat(new, "", &space);
            } else {
                cat(new, value, &space);
            }
            *end = '}'; // set it back to '}
            end++;
            name = end;
        }
    }
    result = 1;
    return result;
}
