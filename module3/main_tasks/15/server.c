#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#include "client_handler.h"  // наша структура состояния и функции обработки

int main(int argc, char *argv[]) {
    int listener, newfd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;
    int portno;
    fd_set master, read_fds; // master – все отслеживаемые сокеты, read_fds – копия для select
    int fd_max;              // максимальный номер дескриптора + 1 для select
    int i;
    client_t clients[MAX_CLIENTS]; // статический массив состояний клиентов

    // Игнорируем SIGPIPE, чтобы не завершиться при записи в закрытый сокет
    signal(SIGPIPE, SIG_IGN);

    // Проверка аргументов
    if (argc < 2) {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    // Шаг 1: создание TCP-сокета
    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    // Разрешаем повторное использование адреса для быстрого перезапуска сервера
    int opt = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Заполняем структуру адреса сервера и выполняем bind (шаг 2)
    bzero(&serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY; // принимать на всех интерфейсах
    serv_addr.sin_port = htons(portno);

    if (bind(listener, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }

    // Шаг 3: начинаем слушать порт, очередь до 5 ожидающих соединений
    listen(listener, 5);

    // Инициализация массива клиентов: -1 означает свободный слот
    for (i = 0; i < MAX_CLIENTS; i++) {
        clients[i].sockfd = -1;
    }

    // Готовим мастер-набор дескрипторов для select
    FD_ZERO(&master);
    FD_SET(listener, &master);  // добавляем слушающий сокет
    fd_max = listener;          // пока максимальный дескриптор – слушающий

    printf("TCP SERVER MULTIPLEX\n");

    // Главный цикл обработки событий
    while (1) {
        // Копируем master
        read_fds = master;
        // ждём готовности чтения на любом из сокетов
        if (select(fd_max + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(1);
        }

        // Проверяем все дескрипторы от 0 до fd_max
        for (i = 0; i <= fd_max; i++) {
            if (FD_ISSET(i, &read_fds)) {  // если дескриптор готов к чтению
                if (i == listener) {
                    // Новое подключение
                    clilen = sizeof(cli_addr);
                    newfd = accept(listener, (struct sockaddr *)&cli_addr, &clilen);
                    if (newfd < 0) {
                        perror("accept");
                        continue;
                    }

                    // Ищем свободный слот в массиве состояний
                    int slot = -1;
                    for (int j = 0; j < MAX_CLIENTS; j++) {
                        if (clients[j].sockfd == -1) {
                            slot = j;
                            break;
                        }
                    }
                    if (slot == -1) {
                        fprintf(stderr, "Too many clients, rejecting\n");
                        close(newfd);
                        continue;
                    }

                    // Инициализируем состояние нового клиента
                    init_client(&clients[slot], newfd);
                    if (clients[slot].sockfd == -1) {
                        continue; // инициализация не удалась
                    }
                    // Добавляем сокет в master и обновляем fd_max
                    FD_SET(newfd, &master);
                    if (newfd > fd_max) fd_max = newfd;

                    // Отправляем клиенту приглашение ввести команду
                    send_msg(newfd, "Enter command (ADD, SUB, MUL, DIV, FILE) or QUIT\r\n");
                    printf("New client (fd=%d)\n", newfd);
                }
                else {
                    // Данные от существующего клиента – ищем его состояние
                    client_t *cli = NULL;
                    for (int j = 0; j < MAX_CLIENTS; j++) {
                        if (clients[j].sockfd == i) {
                            cli = &clients[j];
                            break;
                        }
                    }
                    if (!cli) {
                        // Если не нашли - закрываем сокет и удаляем из master
                        close(i);
                        FD_CLR(i, &master);
                        continue;
                    }

                    // Обрабатываем данные клиента в зависимости от его состояния
                    int keep = handle_client(cli);
                    if (!keep) {
                        // Клиент завершил сессию или произошла ошибка
                        printf("Client fd=%d disconnected\n", i);
                        cleanup_client(cli);          // освобождаем ресурсы
                        FD_CLR(i, &master);           // удаляем сокет из набора
                    }
                }
            }
        }
    }

    close(listener);
    return 0;
}