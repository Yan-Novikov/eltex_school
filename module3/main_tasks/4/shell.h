#ifndef SHELL_H
#define SHELL_H

#define MAX_CMD_LEN 1024
#define MAX_ARGS     64
#define MAX_CMDS     32

char* read_command();
int   execute_pipeline(char *cmdline);

#endif