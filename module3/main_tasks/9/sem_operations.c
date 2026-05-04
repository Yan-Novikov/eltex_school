#include "sem_operations.h"

sem_t *create_or_open_semaphore(unsigned int init_value) {
    sem_t *sem = sem_open(SEM_NAME, O_CREAT, 0644, init_value);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    return sem;
}

void close_and_unlink_semaphore(sem_t *sem) {
    if (sem_close(sem) == -1) {
        perror("sem_close");
    }
    if (sem_unlink(SEM_NAME) == -1) {
        perror("sem_unlink");
    }
}