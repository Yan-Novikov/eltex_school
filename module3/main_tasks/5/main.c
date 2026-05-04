#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "signal_handler.h"

static int fd = -1;

void cleanup(void) {
    if (fd != -1) {
        close(file_fd);
        fd = -1;
    }
}

int main(void) {
    // Открываем файл: только запись, создать, очистить, дописывать в конец
    fd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0644);
    if (fd == -1) {
        perror("open");
        return 1;
    }
    setup_handlers();   // Устанавливаем обработчики сигналов

    char buf[64];
    int len;
    int counter = 0;

    while (1) {
        sleep(1);
        counter++;
        dprintf(fd, "%d\n", counter);   // Запись счётчика в файл
        if(sigint_flag) {
            len = snprintf(buf, sizeof(buf),
                        "Received and handled SIGINT (count=%d)\n",
                        sigint_count);
        } else if (sigquit_flag) {
            len = snprintf(buf, sizeof(buf),
                        "Received and handled SIGQUIT\n");
        }
        if (sigint_flag || sigquit_flag) {
            write(fd, buf, len);
            sigint_flag = 0;
            sigquit_flag = 0;
        }
        if (sigint_count >= 3) {
            cleanup();
            exit(0);
        }
    }

    cleanup();
    close(fd);
    return 0;
}