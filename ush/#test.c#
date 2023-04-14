#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>

int main()
{
  char *line = " prog thi\"s is a s\"ingl\"e a\"rg";
  // char *line = " prog 1 2 3";
  int count = 1;
  int i = 0;
  int j = 0;
  int length = strlen(line);
  bool cond = true;

  while (line[i] != 0 && i < length) {
    if (line[i] != ' ') {
      while (line[i] != 0 && i < length) {
	      if (line[i] == '"') {
          cond = false;
        }
        if (line[i] == ' ') {
          if (cond == false) { // if we have read a \", don't do anything
            ;
          } else {
            printf("break at %d\n", i);
            count++;
            line[i] = 0;
            // i++;
            break;
          }
        }
        i++;
      }
      // line[i] = 0;
      i++;
    } else { // skip leading spaces
      i++;
    }
  }

  printf("count is: %d\n", count);
  // printf("%s\n", line);
}
