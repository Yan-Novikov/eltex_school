#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/wait.h>
#include "sem_operations.h"
#include "file_operations.h"

#define PARENT_SLEEP_SEC 1
#define CHILD_SLEEP_SEC  1

int keep_running = 1;

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

int main() {
    srand(time(NULL));

    // Устанавливаем обработчик SIGINT
    setup_handler();

    // Создание семафор с начальным значением 1
    sem_t *sem = create_or_open_semaphore(1);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Дочерний
        while (keep_running) {
            if (sem_wait(sem) == -1) {
                perror("sem_wait (child)");
                break;
            }

            process_and_clear_file(DATA_FILE);

            if (sem_post(sem) == -1) {
                perror("sem_post (child)");
                break;
            }

            sleep(CHILD_SLEEP_SEC);
        }
        printf("[Child] Exiting.\n");
        exit(EXIT_SUCCESS);
    } else {
        // Родительский
        while (keep_running) {
            if (sem_wait(sem) == -1) {
                perror("sem_wait (parent)");
                break;
            }

            write_random_line(DATA_FILE);

            if (sem_post(sem) == -1) {
                perror("sem_post (parent)");
                break;
            }

            sleep(PARENT_SLEEP_SEC);
        }

        printf("[Parent] Exiting. Waiting for child...\n");

        // Отправляем SIGINT дочернему процессу, чтобы он тоже вышел из цикла
        kill(pid, SIGINT);
        wait(NULL);

        // Закрываем и удаляем семафор
        close_and_unlink_semaphore(sem);
    }

    return 0;
}