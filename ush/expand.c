#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <dirent.h>
#include "defn.h"

//  result of expand

static char* input;
static char* front;
static char* end;
static int space;
// static int result;
static char* newline;

void cat(char* new, char* to_cat, int* space) {
    // printf("space: %d, to_cat: %d, new: %d\n", *space, strlen(to_cat), strlen(new));
    if (strlen(to_cat) + strlen(new) <= *space) {
        strcat(new, to_cat);
        *space -= strlen(to_cat);
    } else {
        fprintf(stderr, "No enough space to add\n");
    }
}

int handle_dollar() {
    end++;
    front = end;
    char pid_str[16] = {0};
    if (sprintf(pid_str, "%d", getpid()) >= 0) {
        cat(newline, pid_str, &space);
        // printf("newline: %s\n", newline);
    } else {
        fprintf(stderr, "failed to get pid");
        return -1;
    }
    return 1;
}

void handle_digit() {
    // printf("args: %d, argc: %d\n", args, arg_count);
    char num[10] = {0};
    if (args > 0) {
        int i = 0;
        while (isdigit(*end)) {
            char n = *end;
            num[i] = n;
            i++;
            end++;
        }
        front = end;
        int pattern_n = atoi(num);
        if (pattern_n >= args) {
            cat(newline, "", &space);
        } else {
            cat(newline, command_line[pattern_n + 1 + shift], &space);
        }
    } else { // interactive mode
        end++;
        front = end;
        if (atoi(num) == 0) {
            cat(newline, "./ush", &space);
        } else {
            cat(newline, "", &space);
        }
    }
}

int handle_pound() {
    char pound[3] = {0};
    if (args > 0) {
        if (sprintf(pound, "%d", args) >= 0) {
            cat(newline, pound, &space);
        } else {
            fprintf(stderr, "failed to get #");
            return -1;
        }
    } else {
        cat(newline, "1", &space);
    }
    end++;
    
    front = end;
    printf("front is at: %c\n", *front);
    return 1;
}

int handle_star() {
    end = (front + 1);
    char* r_express = (front + 1);
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

    if (dir != NULL) {
        bool matched = false;
        if (strchr(r_express, '/') != NULL) {
                fprintf(stderr, "can't include /\n");
                return -1;
        } 
        while ((ent = readdir(dir)) != NULL) {
            if (strcmp(ent->d_name + strlen(ent->d_name) - strlen(r_express), r_express) == 0 
            && ent->d_name[0] != '.') {
                matched = true;
                cat(newline, ent->d_name, &space);
                cat(newline, " ", &space);
            }
        }
        if (matched == false) { //  if we can't find matching files
            cat(newline, "*", &space);
            cat(newline, r_express, &space);
        }
        closedir(dir);
    } else {
        perror("Failed to open directory");
        return -1;
    }
    if (reached_end) { //  get rid of the trailing space
        if (newline[strlen(newline) - 1] == ' ') {
            newline[strlen(newline) - 1] = '\0';
        }
    } else {
        *end = ' ';
    }
    front = end;
    return 1;
}

int expand (char *orig, char *new, int newsize) {
    // need a pointer points to the first char of front
    input = orig;
    front = input;
    int result = 0; 
    // another pointer finds the first '}' and set it to '\0'
    end = input;
    char* value = 0; // the value of the environment variable
    newline = new;
    space = newsize;
    bool has_quote = false; //  if we read a ${, we set it to true

    while (*front != 0 && *end != 0) {
        printf("front is at: %c\n", *front);
        if (*front == '$') {
            end = (front + 1);
            printf("end is at: %c\n", *end);
            if (*end == '$') {
                if (handle_dollar() < 0) {
                    result = -1;
                    return result;
                }
            } else if (*end == '{') {
                front = end + 1;
                has_quote = true;
            } else if (isdigit(*end)) {
                handle_digit();
            } else if (*end == '#') {
                if (handle_pound() < 0) {
                    result = -1;
                    return result;
                }
            } else { //  if we read a $ that is not a ${ or $$, we do nothing
                printf("here\n");
                cat(newline, front, &space);
                return result;
            }
        } else if (*front == '*' && handle_star() < 0) {
            result = -1;
            return result;
        } else if (*front == '\\') {
            if (*(front + 1) == '*') {
                cat(newline, "*", &space);
            }
            front += 2;
        } else if (has_quote == true) {
            while (*end != '}') {
                if (*end == '\0') {
                    fprintf(stderr, "Error: missing '}'\n");
                    result = -1;
                    return result;
                }
                end++;
            }
            has_quote = !has_quote;
            *end = '\0';
            value = getenv(front);
            if (value == NULL) {
                cat(newline, "", &space);
            } else {
                cat(newline, value, &space);
            }
            *end = '}'; // set it back to '}
            end++;
            front = end;
        } else {
            char append[1] = {0};
            append[0] = input[front - input];
            append[1] = '\0';
            cat(newline, append, &space);
            if (*front != ' ' && *(front + 1) == '*') {
                cat(newline, "*", &space);
                front += 2;
            }
            
        }
        front++;
    }
    result = 1;
    return result;
}
