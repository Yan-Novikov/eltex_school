#include "sem_operations.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int init_semaphores(const char *pathname, int proj_id, int create_new) {
    key_t key = ftok(pathname, proj_id);
    if (key == -1) {
        perror("ftok");
        return -1;
    }

    int flags = 0666;
    if (create_new) {
        flags |= IPC_CREAT | IPC_EXCL;
    } else {
        flags |= IPC_CREAT;
    }

    int semid = semget(key, 3, flags);
    if (semid == -1) {
        if (create_new && errno == EEXIST) {
            semid = semget(key, 3, 0666);
        }
        if (semid == -1) {
            perror("semget");
            return -1;
        }
    }
    return semid;
}

int set_semaphore_values(int semid, int empty_val, int full_val, int mutex_val) {
    union semun arg;
    
    arg.val = empty_val;
    if (semctl(semid, 0, SETVAL, arg) == -1) {
        perror("semctl SETVAL empty");
        return -1;
    }
    
    arg.val = full_val;
    if (semctl(semid, 1, SETVAL, arg) == -1) {
        perror("semctl SETVAL full");
        return -1;
    }
    
    arg.val = mutex_val;
    if (semctl(semid, 2, SETVAL, arg) == -1) {
        perror("semctl SETVAL mutex");
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

int sem_V_mutex(int semid, int sem_num) {
    struct sembuf unlock[2] = {
        {sem_num, 0, 0},   // ждать, пока значение станет 0
        {sem_num, 1, 0}    // затем увеличить на 1
    };
    if (semop(semid, unlock, 2) == -1) {
        perror("semop V_mutex");
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