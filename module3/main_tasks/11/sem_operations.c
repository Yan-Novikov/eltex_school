#include "sem_operations.h"


sem_t *create_or_open_semaphore(const char *name, unsigned int init_value) {
    sem_t *sem = sem_open(name, O_CREAT, 0644, init_value);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    return sem;
}

void close_and_unlink_semaphore(sem_t *sem, const char *name) {
    if (sem_close(sem) == -1) {
        perror("sem_close");
    }
    if (sem_unlink(name) == -1) {
        perror("sem_unlink");
    }
}