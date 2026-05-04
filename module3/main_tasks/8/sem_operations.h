#ifndef SEM_OPERATIONS_H
#define SEM_OPERATIONS_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
};

int init_semaphores(const char *pathname, int proj_id, int create_new);
int set_semaphore_values(int semid, int empty_val, int full_val, int mutex_val);

int sem_P(int semid, int sem_num);
int sem_V(int semid, int sem_num);           // обычная V для empty и full
int sem_V_mutex(int semid, int sem_num);     // двухшаговая V для mutex

int remove_semaphores(int semid);

#endif