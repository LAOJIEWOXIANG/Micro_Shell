#include <sys/stat.h>

int expand (char *orig, char *new, int newsize);
int processline (char *line, int infd, int outfd, int flags);
int exec_builtin(char** line, int outfd);
void strmode(mode_t mode, char *p);
extern int args;
extern int shift;
extern int arg_count;
extern char** command_line;
extern int r_value;

#define WAIT 1
#define NO_WAIT 0
#define EXPAND 2
#define NO_EXPAND 0