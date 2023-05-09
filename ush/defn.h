#include <sys/stat.h>

int expand (char *orig, char *new, int newsize);
int exec_builtin(char** line);
void strmode(mode_t mode, char *p);
extern int args;
extern int shift;
extern int arg_count;
extern char** command_line;
extern int r_value;