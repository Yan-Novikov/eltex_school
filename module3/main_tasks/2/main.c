#include "shell.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    char *cmdline;
    char *argv[64];
    int argc;

    while (1) {
        cmdline = read_command();
        if (cmdline == NULL) break;   // Ctrl+D
        argc = parse_command(cmdline, argv);
        if (argc == 0) {
            free(cmdline);
            continue;
        }
        if (execute_command(argv) == -1) {
            free(cmdline);
            break;
        }
        free(cmdline);
    }
    return 0;
}