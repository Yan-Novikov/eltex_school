#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/wait.h>
#include "common.h"
#include "sem_operations.h"
#include "shm_operations.h"

// Глобальные переменные для обработки сигнала
int keep_running = 1;
int shmid = -1;
int semid = -1;
void *shared_addr = NULL;

// Обработчик сигнала SIGINT
void handle_sigint(int sig) {
    if (sig == SIGINT) {
        keep_running = 0;   // при получении Ctrl+C выставляем флаг в 0
    }
}

// Установка обработчика
void setup_handler() {
    signal(SIGINT, handle_sigint);
}

void cleanup() {
    if (shared_addr != NULL) {
        detach_shared_memory(shared_addr);
    }
    if (shmid != -1) {
        remove_shared_memory(shmid);
    }
    if (semid != -1) {
        remove_semaphores(semid);
    }
}

// Генерация случайного набора чисел
void generate_numbers(struct shared_data *data) {
    data->count = rand() % MAX_NUMBERS + 1;
    for (int i = 0; i < data->count; i++) {
        data->numbers[i] = rand() % MAX_VALUE;
    }
    data->data_ready = 1;
    data->result_ready = 0;
}

// Вывод набора и результатов
void print_data(const struct shared_data *data) {
    printf("Набор из %d чисел: ", data->count);
    for (int i = 0; i < data->count; i++) {
        printf("%d ", data->numbers[i]);
    }
    printf("\nМинимум: %d, Максимум: %d\n\n", data->min, data->max);
}

int main(int argc, char *argv[]) {
    const char *pathname = argv[0];

    setup_handler();

    // Инициализация генератора случайных чисел
    srand(time(NULL) ^ getpid());

    // Создание разделяемой памяти
    shmid = init_shared_memory(pathname, SHM_PROJ_ID, sizeof(struct shared_data), 1);
    if (shmid == -1) {
        exit(EXIT_FAILURE);
    }

    // Подключение разделяемой памяти
    shared_addr = attach_shared_memory(shmid);
    if (shared_addr == NULL) {
        remove_shared_memory(shmid);
        exit(EXIT_FAILURE);
    }
    struct shared_data *data = (struct shared_data *) shared_addr;

    // Создание семафоров
    semid = init_semaphores(pathname, SEM_PROJ_ID, 1);
    if (semid == -1) {
        cleanup();
        exit(EXIT_FAILURE);
    }

    // Инициализация семафоров
    if (set_semaphore_values(semid, 0, 0) == -1) {
        cleanup();
        exit(EXIT_FAILURE);
    }

    // Инициализация флагов в разделяемой памяти
    data->data_ready = 0;
    data->result_ready = 0;

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        cleanup();
        exit(EXIT_FAILURE);
    }

    int processed_count = 0;

    if (pid == 0) {
        // Дочерний процесс
        while (keep_running) {
            // P(data_ready)
            if (sem_P(semid, 0) == -1) break;

            if (!keep_running) break;

            if (!data->data_ready) {
                // Такого не должно быть при правильной синхронизации
                continue;
            }

            // Обработка данных: поиск минимума и максимума
            int min = data->numbers[0];
            int max = data->numbers[0];
            for (int i = 1; i < data->count; i++) {
                if (data->numbers[i] < min) min = data->numbers[i];
                if (data->numbers[i] > max) max = data->numbers[i];
            }
            data->min = min;
            data->max = max;
            data->result_ready = 1;
            data->data_ready = 0;  // данные обработаны

            // V(result_ready)
            if (sem_V(semid, 1) == -1) break;
        }
        // Отсоединяем разделяемую память в дочернем процессе
        detach_shared_memory(shared_addr);
        exit(0);
    } else {
        // Родительский процесс
        while (keep_running) {
            // Генерация данных
            generate_numbers(data);

            // V(data_ready)
            if (sem_V(semid, 0) == -1) break;

            // P(result_ready)
            if (sem_P(semid, 1) == -1) break;

            // Если получили сигнал завершения, выходим из цикла
            if (!keep_running) break;

            // Проверяем флаг
            if (data->result_ready) {
                print_data(data);
                processed_count++;
                data->result_ready = 0;
            }

            sleep(1);
        }

        // Ждём завершения дочернего процесса
        kill(pid, SIGINT);
        wait(NULL);

        printf("\nОбработано наборов данных: %d\n", processed_count);

        // Очистка ресурсов
        cleanup();
    }

    return 0;
}