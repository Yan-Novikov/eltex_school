#ifndef COMMON_H
#define COMMON_H

#define MAX_NUMBERS 10         // Максимальное количество чисел в наборе
#define MAX_VALUE 100          // Максимальное значение случайного числа
#define SHM_PROJ_ID 'B'
#define SEM_PROJ_ID 'C'

// Структура, размещаемая в разделяемой памяти
struct shared_data {
    int count;
    int numbers[MAX_NUMBERS];
    int min;
    int max;
    int data_ready;
    int result_ready;
};

#endif