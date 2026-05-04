#ifndef SHELL_H
#define SHELL_H

char* read_command();
int   parse_command(char *cmdline, char *argv[]);
int   execute_command(char *argv[]);

#endif