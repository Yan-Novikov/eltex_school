#ifndef SHM_OPERATIONS_H
#define SHM_OPERATIONS_H

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define SHM_NAME "/shm_random_numbers"
#define MAX_NUMBERS 10

// Структура, размещаемая в разделяемой памяти
typedef struct {
    int numbers[MAX_NUMBERS];
    int count;      // текущее количество чисел в наборе
    int min;        // найденный минимум
    int max;        // найденный максимум
} shared_data_t;

// Создание или открытие объекта разделяемой памяти
int create_or_open_shm(const char *name, size_t size);

// Отображение разделяемой памяти в адресное пространство процесса
shared_data_t *map_shm(int fd, size_t size);

// Закрытие и удаление разделяемой памяти
void close_and_unlink_shm(int fd, shared_data_t *addr, size_t size, const char *name);

#endif