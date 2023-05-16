/* CSCI347 Spring23  
 * Assignment 3
 * Modified April 18, 2023 Yang zheng
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdbool.h>
#include <ctype.h>
#include "defn.h"

/* Constants */ 

#define LINELEN 200000
int args = 0;
int shift = 0;
int arg_count = 0;
char** command_line = NULL;
int r_value = 0;

/* Prototypes */

int processline (char *line, int infd, int outfd, int flags);

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
        if (line[i] != ' ' && line[i] != '\t') {
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
  char** arr = (char**) calloc ((count + 1), sizeof(char*));
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
  // printf("args: %d\n", count);
  arr[count] = NULL;
  *argcptr = count;

  // for (int i = 0; i <= count; i++) {
  //   printf("arr[%d]: %s\n", i, arr[i]);
  // }
  // printf("\n");
  return arr;
}
/* ger rid of leading and trailing spaces */
void off_spaces(char* line) {
  while (*line == ' ') {
    line++;
  }
  if (*line == 0) {
    return;
  }
  char* end = line + strlen(line) - 1;
  while (end > line && *end == ' ') {
    end--;
  }
  *(end + 1) = 0;
}

void handlePipe(char* newLine, int flags) {
  char* subCommand = NULL;
  int status;
  int token_count = 1;
  int pid;
  char *p = newLine;

  while (*p) {
    if (*p == '|') {
        token_count++;
    }
    p++;
  }

  int i = 0;
  int temp;
  subCommand = strtok(newLine, "|");
  // printf("subcommand is: %s\n", subCommand);
  while (subCommand != NULL && i <= token_count) {
    i++;
    int fd[2];
    if (pipe(fd) < 0) {
      perror("pipe");
    }
    off_spaces(subCommand);
    // printf("subcommand is: %s\n", subCommand);
    if (i == 1) {
      pid = processline(subCommand, 0, fd[1], NO_EXPAND | NO_WAIT);
      temp = fd[0];
      close(fd[1]);
    } else if (i < token_count){
      pid = processline(subCommand, temp, fd[1], NO_EXPAND | NO_WAIT);
      temp = fd[0];
      close(fd[0]);
      close(fd[1]);
    } else {
      pid = processline(subCommand, temp, 1, NO_EXPAND | flags);
      close(fd[0]);
    }

    if ((flags & WAIT) != 0) {
      if (pid >= 0 && waitpid(pid, &status, WNOHANG) < 0) {
        fprintf(stderr, "waitpid failed!\n");
      } else {
        // printf("waiting for child to complete\n");
      }
      // printf("waiting for child to complete\n");
    }
    
    subCommand = strtok(NULL, "|");
  }
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
    
    if (*buffer != '\n' && !is_empty_or_spaces(buffer)) {
      /* Get rid of \n at end of buffer. */
      len = strlen(buffer);
      if (buffer[len-1] == '\n') {
          buffer[len-1] = 0;
      }
      // printf("buffer is: %s\n", buffer);
      off_comment(buffer);
      /* Run it ... */
      processline(buffer, 0, 1, EXPAND | WAIT);
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

int processline (char *line, int infd, int outfd, int flags)
{
    pid_t  cpid;
    int    status;
    
    char newLine[LINELEN] = {0};

    if ((flags & EXPAND) != 0) {
      if (expand(line, newLine, LINELEN) == -1) {
        fprintf(stderr, "Expand failed\n");
        return -1;
      }
    } else {
      strcpy(newLine, line);
    }

    if (strchr(newLine, '|') != NULL) {
      handlePipe(newLine, flags);
    }
    // printf("newLine is: %s\n", newLine);
    int argc = 0;
    char** p_arr = arg_parse(newLine, &argc);
    
    /* check if new line contains builtin command before fork */
    if (exec_builtin(p_arr, outfd) < 0) {
      /* Start a new process to do the job. */
      cpid = fork();
      if (cpid < 0) {
        /* Fork wasn't successful */
        perror ("fork");
        return -3;
      }
      
      /* Check for who we are! */
      if (cpid == 0) {
        /* We are the child! */
        if (outfd != 1) {
          dup2(outfd, 1);
        }
        // if (infd != 0) {
        //   dup2(infd, 0);
        // }
        execvp(p_arr[0], p_arr);
        
        /* execlp returned, wasn't successful */
        perror ("exec");
        fclose(stdin);  // avoid a linux stdio bug
        exit (127);
      }

      /* free pointer array */
      free(p_arr);
      p_arr = NULL;
      
      if ((flags & WAIT) != 0) {
        // printf("waiting for child to complete\n");
      /* Have the parent wait for child to complete */
        if (wait (&status) < 0) { //  wait returns the pid or -1
          /* Wait wasn't successful */
          perror ("wait");
        }

        if (WIFEXITED(status)) { // child exited normally
          r_value = WEXITSTATUS(status);
          // printf("child process exited with status %d\n", r_value);
        } else if (WIFSIGNALED(status)) {
          int sig = WTERMSIG(status);
          if (sig == SIGSEGV) {
            printf("Segmentation fault (core dumped)\n");
            fflush(stdout);
          } else if (sig != SIGINT) {
            printf("%s\n", strsignal(sig));
            fflush(stdout);
          }
          r_value = 128 + sig;
        }
        return 0;
      } else {
        return cpid;
      }
    } else {
      return 0;
    }
}





