#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "defn.h"

//  result of expand

static char* input;
static char* front;
static char* end;
static int space;
// static char* newline;

void cat(char* new, char* to_cat, int* space) {
    // printf("space: %d, to_cat: %d, new: %d\n", *space, strlen(to_cat), strlen(new));
    if (strlen(to_cat) + strlen(new) <= *space) {
        strcat(new, to_cat);
        *space -= strlen(to_cat);
    } else {
        fprintf(stderr, "No enough space to add\n");
    }
}

int handle_dollar(char* newline) {
    end++;
    front = end;
    char pid_str[16] = {0};
    if (sprintf(pid_str, "%d", getpid()) >= 0) {
        cat(newline, pid_str, &space);
    } else {
        fprintf(stderr, "failed to get pid\n");
        return -1;
    }
    return 1;
}

void handle_digit(char* newline) {
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

int handle_pound(char* newline) {
    char pound[3] = {0};
    if (args > 0) {
        if (sprintf(pound, "%d", args) >= 0) {
            cat(newline, pound, &space);
        } else {
            fprintf(stderr, "failed to get #\n");
            return -1;
        }
    } else {
        cat(newline, "1", &space);
    }
    end++;
    front = end;
    return 1;
}

int handle_star(char* newline) {
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

int handle_question(char* newline) {
    char p_value[3] = {0};
    if (sprintf(p_value, "%d", r_value) >= 0) {
        cat(newline, p_value, &space);
    } else {
        return -1;
        fprintf(stderr, "failed to get ?\n");
    }
    end++;
    front = end;
    return 1;
}

int check_parent(char* input) {
    int count = 1;

    for (int i = 0; input[i] != '\0'; i++) {
        if (input[i] == '(') {
            count++;
        } else if (input[i] == ')') {
            if (count == 0) {
                return -1; // no matching '('
            }
            count--;
            if (count == 0) {
                return i;
            }
        }
    }
    return -1;
}

void handle_parent(char* orig, char* newline, int last_parent, int newsize) {
    int set_null = front - orig + last_parent;
    orig[set_null] = 0;
    int fd[2];
    int n = 0;
    int buffer_length = strlen(newline);
    char* pass = front;
    if (pipe(fd) < 0) {
        perror("pipe");
    }
    int total_data = strlen(newline);
    // printf("front is %s\n", front);
    int pid = processline(pass, 0, fd[1], EXPAND | NO_WAIT);
    // orig[last_parent] = ')';
    orig[set_null] = ')';
    close(fd[1]); 
    // printf("front is %s\n", front);
    while (total_data < newsize) {
        // if (strlen(newline) == total_data) {
        //     printf("newline is full\n");
        // }
        n = read(fd[0], newline + total_data, newsize - total_data);
        //  read from write end of the pipe to newline
        if (n > 0) {
            total_data += n;
            space -= n;
        } else { //  EOF
            break;
        }
    }
    // printf("front is at %d, %d\n", front - orig, *front);
    // printf("newline is %s\n", newline);
    buffer_length = strlen(newline);
    // printf("buffer_length is %d\n", buffer_length);
    newline[buffer_length] = 0;
    /* turn the \n into spaces except the last one */
    for (int i = 0; i < buffer_length - 1; i++) {
        if (newline[i] == '\n' && newline[i+1] != '\n') {
            newline[i] = ' ';
        }
    }
    
    if (newline[buffer_length - 1] == '\n') {
        newline[buffer_length - 1] = 0;
    }
    
    // printf("built newline is %s, last char is %d\n", newline, newline[(strlen(newline) - 1)]);
    /* wait for child process if there is one */
    int status;
    if (pid < 0) {
        fprintf(stderr, "Error from processline\n");
    } else if (pid > 0 && waitpid(pid, &status, 0) < 0) {
        fprintf(stderr, "waitpid failed!\n");
    }
    if (WIFEXITED(status)) { // child exited normally
        r_value = WEXITSTATUS(status);
        // printf("child process exited with status %d\n", r_value);
    } else if (WIFSIGNALED(status)) {
        int sig = WTERMSIG(status);
        r_value = 128 + sig;
    }
    close(fd[0]);
    // clean up   
}


int expand (char *orig, char *new, int newsize) {
    // need a pointer points to the first char of front
    input = orig;
    front = input;
    int result = 0; 
    // another pointer finds the first '}' and set it to '\0'
    end = input;
    char* value = 0; // the value of the environment variable
    // newline = new;
    space = newsize;
    bool has_quote = false; //  if we read a ${, we set it to true

    while (*front != 0 && *end != 0) {
        // printf("end is %c\n", *end);
        if (*front == '$') {
            end = (front + 1);
            if (*end == '$') {
                if (handle_dollar(new) < 0) {
                    result = -1;
                    return result;
                }
            } else if (*end == '{') {
                front = end + 1;
                has_quote = true;
            } else if (*end == '(') {
                int last_parent = 0;
                front = end + 1; //  front points to the command
                last_parent = check_parent(front);
                // printf("last_parent is %d\n", last_parent);
                if (last_parent < 0) {
                    fprintf(stderr, "Missing )\n");
                    result = -1;
                    return result;
                }
                
                handle_parent(orig, new, last_parent, newsize);
            } else if (isdigit(*end)) {
                handle_digit(new);
            } else if (*end == '#') {
                if (handle_pound(new) < 0) {
                    result = -1;
                    return result;
                }
            } else if (*end == '?') {
                if (handle_question(new) < 0) {
                    result = -1;
                    return result;
                }
            } else { //  if we read a $ that is not a ${ or $$, we do nothing
                cat(new, front, &space);
                return result;
            }
        } else if (*front == '*') {
            if (handle_star(new) < 0) {
                result = -1;
                return result;
            }
        } else if (*front == '\\') {
            if (*(front + 1) == '*') {
                cat(new, "*", &space);
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
                cat(new, "", &space);
            } else {
                cat(new, value, &space);
            }
            *end = '}'; // set it back to '}
            end++;
            front = end;
        } else {
            if (*front == ')') {
                front++;
                // printf("end is %d\n", end - orig);
            }
            char append[1] = {0};
            append[0] = input[front - input];
            append[1] = '\0';
            cat(new, append, &space);
            if (*front != ' ' && *(front + 1) == '*') {
                cat(new, "*", &space);
                front += 2;
            }
            front++;
        }
    }
    result = 1;
    return result;
}
