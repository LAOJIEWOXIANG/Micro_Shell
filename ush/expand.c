#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <dirent.h>
#include "defn.h"

int result = 0; //  result of expand

void cat(char* new, char* to_cat, int* space) {
    // printf("space: %d, to_cat: %d, new: %d\n", *space, strlen(to_cat), strlen(new));
    if (strlen(to_cat) + strlen(new) <= *space) {
        strcat(new, to_cat);
        *space -= strlen(to_cat);
    } else {
        fprintf(stderr, "No enough space to add\n");
    }
}

int expand (char *orig, char *new, int newsize) {
    // need a pointer points to the first char of NAME
    char *name = orig;

    // another pointer finds the first '}' and set it to '\0'
    char *end = orig;

    char* value = 0; // the value of the environment variable
    char pid_str[16] = {0};
    int space = newsize;
    bool has_quote = false; //  if we read a ${, we set it to true

    while (*name != '\0' && *end != '\0') {
        while (*name != '{') {
            if (*name == '\0') { //  if we never read a {
                if (new[strlen(new) - 1] == ' ') {
                    new[strlen(new) - 1] = '\0';
                }
                return result;
            }
            if (*name == '$'){
                name++;
                if (*name == '$') { //  this will increment name
                    if (sprintf(pid_str, "%d", getpid()) >= 0) {
                        cat(new, pid_str, &space);
                    } else {
                        fprintf(stderr, "failed to get pid");
                        result = -1;
                        return result;
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
                        int pattern_n = atoi(num);
                        if (pattern_n >= args) {
                            cat(new, "", &space);
                        } else {
                            cat(new, command_line[pattern_n + 1 + shift], &space); //  out of bounds?
                        }
                        name--;
                    } else { // interactive mode
                        if (atoi(num) == 0) {
                            cat(new, "./ush", &space);
                        } else {
                            cat(new, "", &space);
                        }
                    }
                } else if (*name == '#') {
                    char pound[3] = {0};
                    if (args > 0) {
                        if (sprintf(pound, "%d", args) >= 0) {
                            cat(new, pound, &space);
                        } else {
                            fprintf(stderr, "failed to get #");
                            result = -1;
                            return result;
                        }
                    } else {
                        cat(new, "1", &space);
                    }
                } else { //  if we read a $ that is not a ${ or $$, we do nothing
                    name--;
                    cat(new, name, &space);
                    return result;
                }
            } else if (*name == '*') {
                // name++;
                end = (name + 1);
                char* r_express = (name + 1);
                DIR *dir;
                struct dirent *ent;
                dir = opendir(".");
                bool reached_end = false;
                if (*end == ' ' || *end == '\0') { //  if there is no pattern
                    r_express = "";
                } else {
                    while (*end != ' ' && *end != '\0') {
                        end++;
                    }
                    if (*end == ' ') {
                        *end = '\0';
                    } else {
                        reached_end = true;
                    }
                }
                // r_express = name;
                if (dir != NULL) {
                    while ((ent = readdir(dir)) != NULL) {
                        if (strcmp(ent->d_name + strlen(ent->d_name) - strlen(r_express), r_express) == 0 
                        && ent->d_name[0] != '.') {
                            cat(new, ent->d_name, &space);
                            cat(new, " ", &space);
                        }
                    }
                    closedir(dir);
                } else {
                    perror("Failed to open directory");
                    result = -1;
                    return result;
                }
                if (reached_end) {
                    if (new[strlen(new) - 1] == ' ') {
                    new[strlen(new) - 1] = '\0';
                    }
                    break;
                } else {
                    *end = ' ';
                    name = end;
                }
            } else {
                char append[1] = {0};
                append[0] = orig[name - orig];
                append[1] = '\0';
                cat(new, append, &space);
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
