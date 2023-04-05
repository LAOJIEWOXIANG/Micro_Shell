/* CSCI347 Spring23  
 * Assignment 1
 * Modified April 3, 2023 Yang zheng
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>


/* Constants */ 

#define LINELEN 1024

/* Prototypes */

void processline (char *line);

char** arg_parse (char *line, int *argcptr) {
  int count = 0;
  int i = 0;
  while (line[i] != 0) {
    if (line[i] != ' ') {
      count++;
      while (line[i] != 0 && line[i] != ' ') {
	i++;
      }
      line[i] = 0;
      i++;
    } else {
      i++;
    }
  }
  i = 0;
  int j = 0;
  
  char** arr = (char**) malloc ((count + 1) * sizeof(char*));
  if (arr == NULL) {
     fprintf (stderr, "Failed to malloc");
  }
  
  while (line[i] != 0) {
    if (line[i] != ' ') {
      arr[j] = &line[i];
      j++;
      while (line[i] != 0 && line[i] != ' ') {
	i++;
      }
      line[i] = 0;
      i++;
    } else {
      i++;
    }
  }
  arr[count] = NULL;
  *argcptr = count;
  return arr;
}


/* Shell main */
int
main (void)
{
    char   buffer [LINELEN];    int    len;

    while (1) {

        /* prompt and get line */
	fprintf (stderr, "%% ");
	if (fgets (buffer, LINELEN, stdin) != buffer)
	  break;

        /* Get rid of \n at end of buffer. */
	len = strlen(buffer);
	if (buffer[len-1] == '\n')
	    buffer[len-1] = 0;

	/* Run it ... */
	processline (buffer);

    }

    if (!feof(stdin))
        perror ("read");

    return 0;		/* Also known as exit (0); */
}


void processline (char *line)
{
    pid_t  cpid;
    int    status;

    int argc;
    char** p_arr = arg_parse(line, &argc);
    if (line == NULL) {
      return;
    }
    
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
    p_arr = NULL;
    free(p_arr);
    
    /* Have the parent wait for child to complete */
    if (wait (&status) < 0) {
      /* Wait wasn't successful */
      perror ("wait");
    }
    
}




