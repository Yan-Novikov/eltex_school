#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include "producer_consumer.h"
#include "sem_operations.h"

// Чтение и удаление первой строки из файла
int read_and_remove_first_line(const char *filename, char *buffer, size_t bufsize) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        if (errno == ENOENT) {
            return 0;
        }
        perror("fopen for reading");
        return -1;
    }

    // Читаем первую строку
    if (fgets(buffer, bufsize, fp) == NULL) {
        fclose(fp);
        return 0;
    }

    // Создаём временный файл для оставшихся строк
    char tempname[256];
    snprintf(tempname, sizeof(tempname), "%s.tmp", filename);
    FILE *temp = fopen(tempname, "w");
    if (temp == NULL) {
        perror("fopen temp file");
        fclose(fp);
        return -1;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp) != NULL) {
        fputs(line, temp);
    }

    fclose(fp);
    fclose(temp);

    // Заменяем оригинал временным файлом
    if (rename(tempname, filename) != 0) {
        perror("rename");
        return -1;
    }

    return 1;
}

void process_line(const char *line) {
    int min = INT_MAX;
    int max = INT_MIN;
    int count = 0;

    char *copy = strdup(line);
    char *token = strtok(copy, " \t\n");
    while (token != NULL) {
        int num = atoi(token);
        if (num < min) min = num;
        if (num > max) max = num;
        count++;
        token = strtok(NULL, " \t\n");
    }
    free(copy);

    if (count > 0) {
        printf("Consumer (PID %d): processed line: %s", getpid(), line);
        printf("Consumer (PID %d): min = %d, max = %d\n", getpid(), min, max);
    } else {
        printf("Consumer (PID %d): empty line ignored\n", getpid());
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *filename = argv[1];

    // Подключаемся к существующим семафорам
    int semid = init_semaphores(filename, PROJ_ID, 0);
    if (semid == -1) {
        exit(EXIT_FAILURE);
    }

    printf("Consumer (PID %d) started for file '%s'\n", getpid(), filename);

    while (1) {
        printf("Consumer (PID %d): waiting for full slot...\n", getpid());
        // P(full)
        if (sem_P(semid, 1) == -1) break;

        // P(mutex)
        if (sem_P(semid, 2) == -1) {
            sem_V(semid, 1);
            break;
        }

        // Чтение и удаление первой строки
        char line[256];
        int result = read_and_remove_first_line(filename, line, sizeof(line));
        if (result == -1) {
            sem_V_mutex(semid, 2);
            sem_V(semid, 1);
            break;
        }

        // V(mutex)
        if (sem_V_mutex(semid, 2) == -1) break;

        // V(empty)
        if (sem_V(semid, 0) == -1) break;

        if (result == 1) {
            process_line(line);
        } else {
            printf("Consumer (PID %d): file empty, but full was >0?\n", getpid());
        }

        sleep(1);
    }

    return 0;
}