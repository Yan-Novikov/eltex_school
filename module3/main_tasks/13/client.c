#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

void error(const char *msg) {
    perror(msg);
    exit(0);
}

// функция отправки файла
void send_file(int sock, const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        char err[] = "ERROR: cannot open file\n";
        write(sock, err, strlen(err));
        return;
    }
    fseek(fp, 0, SEEK_END);
    long filesize = ftell(fp);
    rewind(fp);

    char size_buf[32];
    snprintf(size_buf, sizeof(size_buf), "%ld\n", filesize);
    write(sock, size_buf, strlen(size_buf));

    char buffer[1024];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        if (write(sock, buffer, bytes) != (ssize_t)bytes) {
            perror("File send error");
            break;
        }
    }
    fclose(fp);
    printf("File '%s' sent (%ld bytes)\n", filename, filesize);
}

int main(int argc, char *argv[])
{
    int my_sock, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buff[1024];
    static char filename[256] = "";
    int file_mode = 0; // флаг ожидания READY

    printf("TCP DEMO CLIENT\n");

    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    // извлечение порта
    portno = atoi(argv[2]);

    // Шаг 1 - создание сокета
    my_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (my_sock < 0) error("ERROR opening socket");

    // извлечение хоста
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    // заполнение структуры serv_addr
    bzero((char*) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char*)server->h_addr_list[0], (char *)&serv_addr.sin_addr.s_addr, server->h_length);

    // установка порта
    serv_addr.sin_port = htons(portno);

    // Шаг 2 - установка соединения
    if (connect(my_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    // Шаг 3 - чтение и передача сообщений
    while ((n = recv(my_sock, &buff[0], sizeof(buff) - 1, 0)) > 0)
    {
        // ставим завершающий ноль в конце строки
        buff[n] = 0;

        // выводим на экран
        printf("S=>C:%s", buff);

        // проверка на READY от сервера (режим передачи файла)
        if (strncmp(buff, "READY", 5) == 0) {
            send_file(my_sock, filename);
            file_mode = 0;
            continue;
        }

        // если мы в режиме FILE и получили не READY — это ошибка или подтверждение
        if (file_mode) {
            file_mode = 0;
        }

        // читаем пользовательский ввод с клавиатуры
        printf("S<=C:");
        fgets(&buff[0], sizeof(buff) - 1, stdin);

        // проверка на "quit" (оставлено для совместимости, но теперь ещё и QUIT)
        if (!strcmp(buff, "quit\n") || !strcasecmp(buff, "QUIT\n")) {
            // Корректный выход
            send(my_sock, "QUIT\n", 5, 0);
            printf("Exit...\n");
            close(my_sock);
            return 0;
        }

        // обработка команды FILE – сохраняем имя файла и отправляем команду
        if (strncasecmp(buff, "FILE ", 5) == 0) {
            char cmd[32];
            if (sscanf(buff, "%31s %255s", cmd, filename) == 2) {
                file_mode = 1;   // ожидание READY после отправки команды
            }
        }

        // передаем строку клиента серверу
        send(my_sock, &buff[0], strlen(&buff[0]), 0);
    }

    printf("Recv error \n");
    close(my_sock);
    return -1;
}