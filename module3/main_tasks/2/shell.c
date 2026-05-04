#include "shell.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

// Чтение команды
char* read_command() {
    char *cmdline = NULL;
    size_t size = 0;
    printf("> ");
    fflush(stdout);
    getline(&cmdline, &size, stdin);  // getline сам выделит память
    return cmdline;
}

// strtok делит строку на аргументы
int parse_command(char *cmdline, char *argv[]) {
    int argc = 0;
    char *token = strtok(cmdline, " \t\n");
    while (token != NULL) {
        argv[argc++] = token;
        token = strtok(NULL, " \t\n");
    }
    argv[argc] = NULL;
    return argc;
}

int execute_command(char *argv[]) {
    if (strcmp(argv[0], "exit") == 0) {
        printf("Завершение программы.\n");
        return -1;
    }

    pid_t pid = fork();
    if (pid == 0) {
        execvp(argv[0], argv);
        perror("execvp");
        return -1;
    } else {
        wait(NULL);         // ждём любой дочерний процесс
    }
    return 0;
}