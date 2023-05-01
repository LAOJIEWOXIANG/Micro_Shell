/* CSCI347 Spring23  
 * Assignment 2
 * Modified April 11, 2023 Yang zheng
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include "defn.h"

/* Constants */ 

#define LINELEN 1024
int args = 0;
int shift = 0;
int arg_count = 0;
char** command_line = NULL;

/* Prototypes */

void processline (char *line);

void off_quote(char *line) {
  int j = 0;
  int lineLength = strlen(line);
  for (int i = 0; i < lineLength; i++) {
    if (line[i] != '"') {
        line[j++] = line[i];
    }
  }
  line[j] = '\0';
}

/* find the comment and get rid of the comment */
void off_comment(char *line) {
  char* start = line;
  while (*start != '\0') {
    if (*start == '#' && *(start - 1) != '$') {
      *start = '\0';
      break;
    }
    start++;
  }
}

bool is_empty_or_spaces(char *line) {
    int i = 0;
    while (line[i] != '\0') {
        if (line[i] != ' ' && line[i] != '\t' && line[i] != '\n') {
            return false;  // found non-space character, line is not empty or full of spaces
        }
        i++;
    }
    return true;  // end of line reached without finding non-space character, line is empty or full of spaces
}

char** arg_parse (char *line, int *argcptr) {
  int count = 1;
  int i = 0;
  bool no_quote = true;
  int length = strlen(line);

  while (line[i] != 0 && i < length) {
    if (line[i] != ' ') {
      while (line[i] != 0 && i < length) {
	      if (line[i] == '"') {
          no_quote = !no_quote;
        }
        if (line[i] == ' ') {
          if (no_quote == false) { // if we have read a \", don't do anything
            ;
          } else {
            count++;
            break;
          }
        }
        i++;
      }
      i++;
    } else {
      i++;
    }
  }

  if (no_quote == false) {
    fprintf(stderr, "No matching double quotes");
  }

  i = 0;
  int j = 0;
  char** arr = (char**) malloc ((count + 1) * sizeof(char*));
  if (arr == NULL) {
     fprintf (stderr, "Failed to malloc");
  }
  
  while (line[i] != 0 && i < length) {
    if (line[i] != ' ') {
      arr[j] = &line[i];
      j++;
      while (line[i] != 0 && i < length) {
	      if (line[i] == '"') {
          no_quote = !no_quote;
        }
        if (line[i] == ' ') {
          if (no_quote == false) { // if we have read a \", don't do anything
            ;
          } else {
            line[i] = 0;
            break;
          }
        }
        i++;
      }
      i++;
    } else {
      i++;
    }
  }

  for (int i = 0; i < j; i++) {
    off_quote(arr[i]);
  }

  arr[count] = NULL;
  *argcptr = count;
  return arr;
}


/* Shell main */
int
main (int argc, char **argv)
{
  // for (int i = 0; i < argc; i++) {
  //   printf("argv[%d]: %s\n", i, argv[i]);
  // }
  arg_count = argc - 1;
  args = argc - 1; //  args starts from index 2 to index n - 1 of the command line
  command_line = argv;
  char buffer[LINELEN];
  int len;
  FILE* read;
  if (argc == 1) {
    read = stdin;
  } else {
    // char* filename = argv[1];
    read = fopen(argv[1], "r");
    if (read == NULL) {
      fprintf(stderr, "Failed to open file %s\n", argv[1]);
      exit(127);
    }
  }
  while (1) {

    /* prompt and get line */
    if (read == stdin) {
      fprintf (stderr, "%% ");
    }

    if (fgets (buffer, LINELEN, read) != buffer) {
      break;
    }

    if (!is_empty_or_spaces(buffer)) {
      /* Get rid of \n at end of buffer. */
      // printf("buffer: %s\n", buffer);
      len = strlen(buffer);
      if (buffer[len-1] == '\n')
          buffer[len-1] = 0;
      off_comment(buffer);
      /* Run it ... */
      processline (buffer);
    }
    if (feof(read)) {
      break;
    }
  }

  if (!feof(read)) {
    perror ("read");
  }

  fclose(read);
  
  return 0;		/* Also known as exit (0); */ 
}


void processline (char *line)
{
    pid_t  cpid;
    int    status;

    char newLine[LINELEN] = {0};
    int condition = expand(line, newLine, LINELEN);
    if (condition == -1) { // if expand failed, print error message
      fprintf(stderr, "Expand failed\n");
      return;
    }

    int argc = 0;
    char** p_arr = arg_parse(newLine, &argc);
    // printf("p_arr[0]: %s\n", p_arr[0]);
    if (newLine == NULL || p_arr[0] == NULL) {
      // printf("p_arr is NULL\n");
      return;
    }
    
    /* check if new line contains builtin command before fork */
    if (exec_builtin(p_arr) < 0) {
      /* Start a new process to do the job. */
      cpid = fork();
      if (cpid < 0) {
        /* Fork wasn't successful */
        perror ("fork");
        return;
      }
      
      /* Check for who we are! */
      if (cpid == 0) {
        /* We are the child! */
        execvp(p_arr[0], p_arr);
        
        /* execlp reurned, wasn't successful */
        perror ("exec");
        fclose(stdin);  // avoid a linux stdio bug
        exit (127);
      }

      /* free pointer array */
      free(p_arr);
      p_arr = NULL;
      
      /* Have the parent wait for child to complete */
      if (wait (&status) < 0) {
        /* Wait wasn't successful */
        perror ("wait");
      }
    } else {
      ;
    }
    
}





