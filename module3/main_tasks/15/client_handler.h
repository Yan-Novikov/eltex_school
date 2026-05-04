#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>

#define MAX_CLIENTS 1024

typedef enum {
    STATE_CMD,
    STATE_PARAM1,
    STATE_PARAM2,
    STATE_FILE_SIZE,
    STATE_FILE_DATA
} client_state_t;

typedef struct {
    int sockfd;
    FILE *stream;           // поток для построчного чтения
    client_state_t state;
    char op;
    int a, b;
    char filename[256];
    long filesize;
    long bytes_received;
    FILE *fp;               // файл для записи
} client_t;

void init_client(client_t *cli, int sock);
void cleanup_client(client_t *cli);
int send_msg(int sock, const char *msg);
int handle_client(client_t *cli);

#endif