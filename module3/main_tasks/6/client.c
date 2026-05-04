#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>

static void receiver(int msqid, int client_id) {
    struct chat_msg msg;
    while (msgrcv(msqid, &msg, sizeof(msg) - sizeof(long), client_id, 0) != -1) {
        printf("\n[От %d]: %s\n> ", msg.sender_id, msg.text);
        fflush(stdout);
    }
    exit(0);
}

static void sigint_handler(int sig) {
    exit(0);
}

void run_client(int msqid, int client_id) {
    signal(SIGINT, sigint_handler);

    // Отправляем служебное сообщение для регистрации на сервере
    struct chat_msg join_msg = { .mtype = SERVER_TYPE, .sender_id = client_id };
    strcpy(join_msg.text, "__JOIN__");
    if (msgsnd(msqid, &join_msg, sizeof(join_msg) - sizeof(long), 0) == -1) {
        perror("msgsnd __JOIN__");
        exit(1);
    }
    
    printf("Клиент %d подключён. 'shutdown' для выхода.\n", client_id);

    pid_t pid = fork();
    if (pid == 0) receiver(msqid, client_id);

    struct chat_msg msg = { .mtype = SERVER_TYPE, .sender_id = client_id };
    char input[TEXT_SIZE];
    while (1) {
        printf("> ");
        fflush(stdout);
        if (!fgets(input, TEXT_SIZE, stdin)) break;
        input[strcspn(input, "\n")] = 0;
        if (!*input) continue;

        strncpy(msg.text, input, TEXT_SIZE - 1);
        msg.text[TEXT_SIZE - 1] = 0;
        msgsnd(msqid, &msg, sizeof(msg) - sizeof(long), 0);

        if (!strcmp(input, "shutdown")) break;
    }
    kill(pid, SIGTERM);
    wait(NULL);
}