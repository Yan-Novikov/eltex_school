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

// Создание или подключение к набору семафоров (2 семафора: data_ready, result_ready)
int init_semaphores(const char *pathname, int proj_id, int create_new);

// Установка начальных значений семафоров
int set_semaphore_values(int semid, int data_ready_val, int result_ready_val);

// Операции P и V
int sem_P(int semid, int sem_num);
int sem_V(int semid, int sem_num);

// Удаление набора семафоров
int remove_semaphores(int semid);

#endif