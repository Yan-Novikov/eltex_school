#ifndef SHM_OPERATIONS_H
#define SHM_OPERATIONS_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

// Создание или подключение к сегменту разделяемой памяти
int init_shared_memory(const char *pathname, int proj_id, size_t size, int create_new);

// Присоединение сегмента к адресному пространству процесса
void *attach_shared_memory(int shmid);

// Отсоединение сегмента
int detach_shared_memory(const void *shmaddr);

// Удаление сегмента разделяемой памяти
int remove_shared_memory(int shmid);

#endif