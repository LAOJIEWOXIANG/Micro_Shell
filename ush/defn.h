int expand (char *orig, char *new, int newsize);
int exec_builtin(char** line);
extern int args;
extern int shift;
extern int arg_count;
extern char** command_line;