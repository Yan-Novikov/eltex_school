#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include "producer_consumer.h"
#include "sem_operations.h"

#define MAX_NUMBERS 10      // Максимальное количество чисел в строке
#define MAX_NUMBER_VALUE 100

void generate_random_string(char *buffer, size_t size) {
    int count = rand() % MAX_NUMBERS + 1;
    int pos = 0;
    for (int i = 0; i < count; i++) {
        int num = rand() % MAX_NUMBER_VALUE;
        pos += snprintf(buffer + pos, size - pos, "%d ", num);
    }
    if (pos > 0) {
        buffer[pos - 1] = '\n';
        buffer[pos] = '\0';
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *filename = argv[1];
    srand(time(NULL) ^ getpid());

    // Создаём или подключаемся к семафорам
    int semid = init_semaphores(filename, PROJ_ID, 1);
    if (semid == -1) {
        exit(EXIT_FAILURE);
    }

    // Проверим значение empty: если 0, значит семафоры ещё не инициализированы
    int empty_val = semctl(semid, 0, GETVAL);
    if (empty_val == -1) {
        perror("semctl GETVAL");
        exit(EXIT_FAILURE);
    }
    if (empty_val == 0) {
        // Новый набор, устанавливаем начальные значения
        if (set_semaphore_values(semid, MAX_LINES, 0, 1) == -1) {
            exit(EXIT_FAILURE);
        }
        printf("Producer (PID %d): initialized semaphores for file '%s'\n", getpid(), filename);
    }

    printf("Producer (PID %d) started for file '%s'\n", getpid(), filename);

    while (1) {
        // Генерируем строку
        char line[256];
        generate_random_string(line, sizeof(line));

        printf("Producer (PID %d): waiting for empty slot...\n", getpid());
        // P(empty)
        if (sem_P(semid, 0) == -1) break;

        // P(mutex)
        if (sem_P(semid, 2) == -1) {
            sem_V(semid, 0);
            break;
        }

        // Запись в файл
        FILE *fp = fopen(filename, "a");
        if (fp == NULL) {
            perror("fopen");
            sem_V_mutex(semid, 2);
            sem_V(semid, 0);
            break;
        }
        fputs(line, fp);
        fclose(fp);
        printf("Producer (PID %d): wrote line: %s", getpid(), line);

        // V(mutex)
        if (sem_V_mutex(semid, 2) == -1) break;

        // V(full)
        if (sem_V(semid, 1) == -1) break;

        sleep(1);
    }

    return 0;
}