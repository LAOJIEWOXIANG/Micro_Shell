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
  int l = strlen(line);
  char copy[l];
  int j = 0;
  for (int i = 0; i < l; i++) {
    if (line[i] != '#') {
      copy[j] = line[i];
      j++;
    } else if (line[i] == '#' && line[i - 1] == '$') {
      copy[j] = line[i];
      j++;
    } else { // find the comment, skip the comment
      while (line[i] != ' ') {
        i++;
      }
    }
  }
  copy[j] = '\0';
  strcpy(line, copy);
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
  fprintf (stderr, "%% ");


  if (fgets (buffer, LINELEN, read) != buffer)
    break;

  /* Get rid of \n at end of buffer. */
  len = strlen(buffer);
  if (buffer[len-1] == '\n')
      buffer[len-1] = 0;
  off_comment(buffer);
  /* Run it ... */
  processline (buffer);
  }

  if (!feof(stdin))
    perror ("read");

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
    // printf("newLine: %s\n", newLine);
    int argc = 0;
    char** p_arr = arg_parse(newLine, &argc);
    if (newLine == NULL) {
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





