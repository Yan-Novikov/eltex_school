#include "shm_operations.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int init_shared_memory(const char *pathname, int proj_id, size_t size, int create_new) {
    key_t key = ftok(pathname, proj_id);
    if (key == -1) {
        perror("ftok (shm)");
        return -1;
    }

    int flags = 0666;
    if (create_new) {
        flags |= IPC_CREAT | IPC_EXCL;
    } else {
        flags |= IPC_CREAT;
    }

    int shmid = shmget(key, size, flags);
    if (shmid == -1) {
        if (create_new && errno == EEXIST) {
            shmid = shmget(key, size, 0666);
        }
        if (shmid == -1) {
            perror("shmget");
            return -1;
        }
    }
    return shmid;
}

void *attach_shared_memory(int shmid) {
    void *addr = shmat(shmid, NULL, 0);
    if (addr == (void *) -1) {
        perror("shmat");
        return NULL;
    }
    return addr;
}

int detach_shared_memory(const void *shmaddr) {
    if (shmdt(shmaddr) == -1) {
        perror("shmdt");
        return -1;
    }
    return 0;
}

int remove_shared_memory(int shmid) {
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl IPC_RMID");
        return -1;
    }
    return 0;
}