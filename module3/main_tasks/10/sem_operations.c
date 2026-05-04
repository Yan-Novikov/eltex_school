#include "sem_operations.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int init_semaphores(const char *pathname, int proj_id, int create_new) {
    key_t key = ftok(pathname, proj_id);
    if (key == -1) {
        perror("ftok (sem)");
        return -1;
    }

    int flags = 0666;
    if (create_new) {
        flags |= IPC_CREAT | IPC_EXCL;
    } else {
        flags |= IPC_CREAT;
    }

    int semid = semget(key, 2, flags);  // 2 семафора
    if (semid == -1) {
        if (create_new && errno == EEXIST) {
            semid = semget(key, 2, 0666);
        }
        if (semid == -1) {
            perror("semget");
            return -1;
        }
    }
    return semid;
}

int set_semaphore_values(int semid, int data_ready_val, int result_ready_val) {
    union semun arg;
    
    // Семафор 0: data_ready
    arg.val = data_ready_val;
    if (semctl(semid, 0, SETVAL, arg) == -1) {
        perror("semctl SETVAL data_ready");
        return -1;
    }
    
    // Семафор 1: result_ready
    arg.val = result_ready_val;
    if (semctl(semid, 1, SETVAL, arg) == -1) {
        perror("semctl SETVAL result_ready");
        return -1;
    }
    
    return 0;
}

int sem_P(int semid, int sem_num) {
    struct sembuf sb = {sem_num, -1, 0};
    if (semop(semid, &sb, 1) == -1) {
        perror("semop P");
        return -1;
    }
    return 0;
}

int sem_V(int semid, int sem_num) {
    struct sembuf sb = {sem_num, 1, 0};
    if (semop(semid, &sb, 1) == -1) {
        perror("semop V");
        return -1;
    }
    return 0;
}

int remove_semaphores(int semid) {
    if (semctl(semid, 0, IPC_RMID) == -1) {
        perror("semctl IPC_RMID");
        return -1;
    }
    return 0;
}