#include "proc.h"
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>


static void print_result(char* strok, char* who){
    char *endptr;
    double number = strtod(strok, &endptr);
    if (endptr != strok && *endptr == '\0') {
        printf("%s: Number: %lf and 2x number: %lf\n", who, number, number * 2);
    } else {
        printf("%s: %s\n", who, strok);
    }
}

static void arg_walker(int argc, char* argv[], char* who){
    int rule_flag = 0;
    if(who == "Parent") rule_flag = 1;
    for(int i = 1; i < argc; i++){
        if(i % 2 == rule_flag) continue;
        print_result(argv[i], who);
    }
}

void arg_processing(int argc, char* argv[]){
    pid_t pid;
    int rv;
    switch(pid = fork()){
        case -1:
            perror("fork");
            exit(EXIT_FAILURE);
        case 0: // Потомок
            arg_walker(argc, argv, "Child");
            _exit(EXIT_SUCCESS);
        default: // Родитель
            arg_walker(argc, argv, "Parent");
            wait(&rv);
            printf("rv = %d. Дочерний процесс успешно завершился.\n", rv);
            exit(EXIT_SUCCESS);
    }
}