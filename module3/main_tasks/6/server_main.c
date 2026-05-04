#include "server.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    key_t key = ftok(KEY_PATH, 'A');
    if (key == -1) {
        perror("ftok");
        exit(1);
    }
    int msqid = get_queue(key, 1);
    printf("Сервер запущен, msqid=%d\n", msqid);
    run_server(msqid);
    return 0;
}