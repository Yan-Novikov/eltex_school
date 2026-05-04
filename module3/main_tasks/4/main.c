#include "shell.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
    char *cmdline;
    while (1) {
        cmdline = read_command();
        if (cmdline == NULL) {   // Ctrl+D
            printf("\nВыход.\n");
            break;
        }
        if (strlen(cmdline) == 0)
            continue;
        if (strcmp(cmdline, "exit") == 0) {
            printf("Завершение программы.\n");
            break;
        }
        execute_pipeline(cmdline);
    }
    return 0;
}