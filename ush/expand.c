#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "defn.h"

int expand (char *orig, char *new, int newsize) {
    // need a pointer points to the first char of NAME
    char *name = orig;
    // another pointer finds the first '}' and set it to '\0'
    char *end = orig;
    int result = 0;

    while (*name != '\0' && *end != '\0') {
        while (*name != '{') {
            if (*name == '\0') {
                return result;
            }
            name++;
        }
        name++;
        // printf("FRONT is %c\n", *name);
        //set the last char of orig to '\0', now name points to a string
        while (*end != '}') {
            if (*end == '\0') {
                fprintf(stderr, "Error: missing '}'\n");
                result = -1;
                return result;
            }
            end++;
        }
        printf("end is %c\n", *end);
        *end = '\0';
        printf("variable name is %s\n", name);

        //  if we don't have enough space to store the value, 
        if (strlen(new) > newsize) {
            fprintf(stderr, "Error: not enough space to store the value");
            result = -1;
            return result;
        } else {
            new = getenv(name);
            printf("value is %s\n", new);
            newsize = newsize - strlen(new);
            // pass name to get environment variable value
        }
        *end = '}'; // set it back to '}
        // printf("line is %s\n", orig);
    }
    result = 1;
    return result;
}