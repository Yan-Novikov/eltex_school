#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/wait.h>
#include "shm_operations.h"
#include "sem_operations.h"

#define PARENT_SLEEP_US 500000

int keep_running = 1;

// Простейший обработчик SIGINT
void handle_sigint(int sig) {
    if (sig == SIGINT) {
        keep_running = 0;
    }
}

int main() {
    srand(time(NULL));

    // Устанавливаем обработчик SIGINT
    signal(SIGINT, handle_sigint);

    size_t shm_size = sizeof(shared_data_t);
    int shm_fd = create_or_open_shm(SHM_NAME, shm_size);
    shared_data_t *shm = map_shm(shm_fd, shm_size);

    sem_t *sem_parent_written = create_or_open_semaphore(SEM_PARENT_WRITTEN, 0);
    sem_t *sem_child_processed = create_or_open_semaphore(SEM_CHILD_PROCESSED, 0);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Дочерний
        // Игнорируем SIGINT в дочернем процессе, чтобы он не завершался раньше времени
        signal(SIGINT, SIG_IGN);

        while (1) {
            if (sem_wait(sem_parent_written) == -1) {
                break;
            }

            // Находим min и max
            int min = shm->numbers[0];
            int max = shm->numbers[0];
            for (int i = 1; i < shm->count; i++) {
                if (shm->numbers[i] < min) min = shm->numbers[i];
                if (shm->numbers[i] > max) max = shm->numbers[i];
            }

            shm->min = min;
            shm->max = max;

            // Сообщаем родителю о завершении обработки
            if (sem_post(sem_child_processed) == -1) {
                break;
            }
        }
        printf("[Child] Exiting.\n");
        exit(EXIT_SUCCESS);
    } else {
        // Родительский
        int processed_count = 0;

        while (keep_running) {
            // Генерация случайных данных
            int count = rand() % MAX_NUMBERS + 1;
            shm->count = count;
            for (int i = 0; i < count; i++) {
                shm->numbers[i] = rand() % 100;
            }

            printf("Set %d: ", ++processed_count);
            for (int i = 0; i < count; i++) {
                printf("%d", shm->numbers[i]);
                if (i < count - 1) printf(", ");
            }
            printf("\n");

            // Отправляем сигнал дочернему, что данные готовы
            if (sem_post(sem_parent_written) == -1) {
                perror("sem_post (parent)");
                break;
            }

            if (sem_wait(sem_child_processed) == -1) {
                break;
            }

            printf("count=%d, min=%d, max=%d\n",
                   shm->count, shm->min, shm->max);

            usleep(PARENT_SLEEP_US);
        }

        printf("[Parent] Exiting. Processed sets: %d\n", processed_count);

        kill(pid, SIGTERM);
        wait(NULL);

        // Освобождаем ресурсы
        close_and_unlink_semaphore(sem_parent_written, SEM_PARENT_WRITTEN);
        close_and_unlink_semaphore(sem_child_processed, SEM_CHILD_PROCESSED);
        close_and_unlink_shm(shm_fd, shm, shm_size, SHM_NAME);
    }

    return 0;
}