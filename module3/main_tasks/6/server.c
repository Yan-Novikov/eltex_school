#include "server.h"
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>

#define MAX_CLIENTS 100

static int clients[MAX_CLIENTS];
static int num_clients = 0;
int server_msqid;

static void add_client(int id) {
    for (int i = 0; i < num_clients; ++i)
        if (clients[i] == id) return;
    if (num_clients < MAX_CLIENTS)
        clients[num_clients++] = id;
}

static void remove_client(int id) {
    for (int i = 0; i < num_clients; ++i) {
        if (clients[i] == id) {
            clients[i] = clients[--num_clients];
            return;
        }
    }
}

static void sigint_handler(int sig) {
    (void)sig;
    msgctl(server_msqid, IPC_RMID, NULL);
    exit(0);
}

void run_server(int msqid) {
    struct chat_msg msg;
    server_msqid = msqid;
    signal(SIGINT, sigint_handler);

    while (1) {
        if (msgrcv(msqid, &msg, sizeof(msg) - sizeof(long), SERVER_TYPE, 0) == -1) {
            perror("msgrcv");
            break;
        }
        int sender = msg.sender_id;
        if (strcmp(msg.text, "__JOIN__") == 0) {
            add_client(sender);
            printf("Клиент %d присоединился к чату.\n", sender);
            continue;
        }
        if (strcmp(msg.text, "shutdown") == 0) {
            remove_client(sender);
            continue;
        }

        add_client(sender);

        struct chat_msg fwd = { .sender_id = sender };
        strncpy(fwd.text, msg.text, TEXT_SIZE - 1);
        fwd.text[TEXT_SIZE - 1] = '\0';

        for (int i = 0; i < num_clients; ++i) {
            int target = clients[i];
            if (target == sender) continue;
            fwd.mtype = target;
            msgsnd(msqid, &fwd, sizeof(fwd) - sizeof(long), 0);
        }
    }
    msgctl(msqid, IPC_RMID, NULL);
}