#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

// функция обслуживания подключившихся пользователей
void dostuff(int);

// функция обработки ошибок
void error(const char *msg) {
    perror(msg);
    exit(1);
}

// количество активных пользователей
int nclients = 0;

// печать количества активных пользователей
void printusers() {
    if (nclients) {
        printf("%d user on-line\n", nclients);
    }
    else {
        printf("No User on line\n");
    }
}

// myfunc теперь принимает оператор и обрабатывает + - * /
int myfunc(char op, int a, int b) {
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/':
            if (b == 0) return 0;
            return a / b;
        default:  return a + b;
    }
}

int main(int argc, char *argv[])
{
    char buff[1024];                // Буфер для различных нужд
    int sockfd, newsockfd;          // дескрипторы сокетов
    int portno;                     // номер порта
    int pid;                        // id номер потока
    socklen_t clilen;               // размер адреса клиента типа socklen_t
    struct sockaddr_in serv_addr, cli_addr; // структура сокета сервера и клиента

    printf("TCP SERVER\n");

    // ошибка в случае если мы не указали порт
    if (argc < 2) {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    // Шаг 1 - создание сокета
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");

    // Шаг 2 - связывание сокета с локальным адресом
    bzero((char*) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY; // сервер принимает подключения на все IP-адреса
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    // Шаг 3 - ожидание подключений, размер очереди - 5
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    // Шаг 4 - извлекаем сообщение из очереди (цикл извлечения запросов на подключение)
    while (1)
    {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) error("ERROR on accept");
        nclients++;

        // вывод сведений о клиенте
        struct hostent *hst;
        hst = gethostbyaddr((char *)&cli_addr.sin_addr, 4, AF_INET);
        printf("+%s [%s] new connect!\n",
               (hst) ? hst->h_name : "Unknown host",
               inet_ntoa(cli_addr.sin_addr));
        printusers();

        pid = fork();
        if (pid < 0) error("ERROR on fork");
        if (pid == 0) {
            close(sockfd);
            dostuff(newsockfd);
            exit(0);
        }
        else close(newsockfd);
    }
    close(sockfd);
    return 0;
}

void dostuff(int sock) {
    int bytes_recv;                 // размер принятого сообщения
    int a, b;                       // переменные для myfunc
    char buff[20 * 1024];
    char cmd[32];                   // буфер для команды

    // приглашение к вводу команды
    #define str_cmd "Enter command (ADD, SUB, MUL, DIV, FILE) or QUIT\r\n"
    write(sock, str_cmd, sizeof(str_cmd));

    // чтение команды
    bytes_recv = read(sock, buff, sizeof(buff)-1);
    if (bytes_recv <= 0) {
        nclients--;
        printf("-disconnect\n");
        printusers();
        close(sock);
        return;
    }
    buff[bytes_recv] = '\0';
    buff[strcspn(buff, "\r\n")] = '\0';   // убираем символы перевода строки

    if (sscanf(buff, "%31s", cmd) != 1) {
        char *err = "ERROR: invalid command\r\n";
        write(sock, err, strlen(err));
        nclients--;
        printf("-disconnect\n");
        printusers();
        close(sock);
        return;
    }

    // обработка QUIT
    if (strcasecmp(cmd, "QUIT") == 0) {
        char *bye = "Goodbye!\n";
        write(sock, bye, strlen(bye));
        nclients--;
        printf("-disconnect\n");
        printusers();
        close(sock);
        return;
    }

    // обработка передачи файла
    if (strcasecmp(cmd, "FILE") == 0) {
        char filename[256];
        if (sscanf(buff, "%*s %255s", filename) != 1) {
            char *err = "ERROR: specify filename\r\n";
            write(sock, err, strlen(err));
            nclients--;
            printf("-disconnect\n");
            printusers();
            close(sock);
            return;
        }

        write(sock, "READY\r\n", 7);

        bzero(buff, sizeof(buff));
        bytes_recv = read(sock, buff, sizeof(buff)-1);
        if (bytes_recv <= 0) {
            nclients--;
            printf("-disconnect\n");
            printusers();
            close(sock);
            return;
        }
        long filesize = atol(buff);

        FILE *fp = fopen(filename, "wb");
        if (!fp) {
            char *err = "ERROR: cannot create file\r\n";
            write(sock, err, strlen(err));
            nclients--;
            printf("-disconnect\n");
            printusers();
            close(sock);
            return;
        }

        long remaining = filesize;
        while (remaining > 0) {
            int to_read = (remaining < sizeof(buff)) ? remaining : sizeof(buff);
            bytes_recv = read(sock, buff, to_read);
            if (bytes_recv <= 0) break;
            fwrite(buff, 1, bytes_recv, fp);
            remaining -= bytes_recv;
        }
        fclose(fp);

        write(sock, "OK: file received\r\n", 19);
        printf("File '%s' received (%ld bytes)\n", filename, filesize);

        nclients--;
        printf("-disconnect\n");
        printusers();
        close(sock);
        return;
    }

    // определяем оператор по команде
    char op;
    if (strcasecmp(cmd, "ADD") == 0) op = '+';
    else if (strcasecmp(cmd, "SUB") == 0) op = '-';
    else if (strcasecmp(cmd, "MUL") == 0) op = '*';
    else if (strcasecmp(cmd, "DIV") == 0) op = '/';
    else {
        char *err = "ERROR: unknown command\r\n";
        write(sock, err, strlen(err));
        nclients--;
        printf("-disconnect\n");
        printusers();
        close(sock);
        return;
    }

    #define str1 "Enter 1 parameter\r\n"
    #define str2 "Enter 2 parameter\r\n"

    // отправляем клиенту сообщение
    write(sock, str1, sizeof(str1));

    // обработка первого параметра
    bytes_recv = read(sock, &buff[0], sizeof(buff));
    if (bytes_recv < 0) error("ERROR reading from socket");
    a = atoi(buff);                 // преобразование первого параметра в int

    // отправляем клиенту сообщение
    write(sock, str2, sizeof(str2));
    bytes_recv = read(sock, &buff[0], sizeof(buff));
    if (bytes_recv < 0) error("ERROR reading from socket");
    b = atoi(buff);                 // преобразование второго параметра в int

    a = myfunc(op, a, b);

    snprintf(buff, sizeof(buff), "%d", a); // преобразование результата в строку
    strcat(buff, "\n");

    // отправляем клиенту результат
    write(sock, &buff[0], strlen(buff));

    nclients--;                     // уменьшаем счетчик активных клиентов
    printf("-disconnect\n");
    printusers();
    close(sock);
    return;
}