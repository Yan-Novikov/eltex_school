#include "file_operations.h"

void write_random_line(const char *filename) {
    FILE *fp = fopen(filename, "a");
    if (fp == NULL) {
        perror("fopen for append");
        return;
    }

    // Генерация случайного количества чисел от 1 до 10
    int count = rand() % 10 + 1;
    for (int i = 0; i < count; i++) {
        int num = rand() % 100;  // числа от 0 до 99
        fprintf(fp, "%d", num);
        if (i < count - 1)
            fprintf(fp, " ");
    }
    fprintf(fp, "\n");
    fclose(fp);
}

void process_and_clear_file(const char *filename) {
    FILE *fp = fopen(filename, "r+");
    if (fp == NULL) {
        // Нет файла - return
        return;
    }

    char line[1024];
    int line_num = 0;
    // Чтение и обработка всех строк
    while (fgets(line, sizeof(line), fp) != NULL) {
        line_num++;
        //  Замена \n на \0
        line[strcspn(line, "\n")] = '\0';

        int min = INT_MAX;
        int max = INT_MIN;
        int value;
        char *token = strtok(line, " ");
        while (token != NULL) {
            value = atoi(token);
            if (value < min) min = value;
            if (value > max) max = value;
            token = strtok(NULL, " ");
        }

        if (min != INT_MAX && max != INT_MIN) {
            printf("[Child] Line %d: min = %d, max = %d\n", line_num, min, max);
        }
    }

    // Очиcтка файла
    if (freopen(filename, "w", fp) == NULL) {
        perror("freopen for truncate");
    }
    fclose(fp);
}