#ifndef SEM_OPERATIONS_H
#define SEM_OPERATIONS_H

#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

#define SEM_NAME "/sem_file_access"

sem_t *create_or_open_semaphore(unsigned int init_value);
void close_and_unlink_semaphore(sem_t *sem);

#endif