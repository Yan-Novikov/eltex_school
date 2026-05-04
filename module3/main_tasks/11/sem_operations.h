#ifndef SEM_OPERATIONS_H
#define SEM_OPERATIONS_H

#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

// Имена семафоров
#define SEM_PARENT_WRITTEN "/sem_parent_written"
#define SEM_CHILD_PROCESSED "/sem_child_processed"

// Создание или открытие семафора
sem_t *create_or_open_semaphore(const char *name, unsigned int init_value);

// Закрытие и удаление семафора
void close_and_unlink_semaphore(sem_t *sem, const char *name);

#endif