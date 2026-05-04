#include "common.h"
#include <stdio.h>
#include <stdlib.h>

int get_queue(key_t key, int create) {
    int flags = 0666;
    if (create) flags |= IPC_CREAT;
    int id = msgget(key, flags);
    if (id == -1) {
        perror("msgget");
        exit(1);
    }
    return id;
}