#include "client_handler.h"

// Инициализация нового клиента: создаём поток для сокета, сбрасываем состояние
void init_client(client_t *cli, int sock) {
    cli->sockfd = sock;
    // fdopen связывает файловый поток с сокетом для удобного построчного ввода
    cli->stream = fdopen(sock, "r+");
    if (!cli->stream) {
        perror("fdopen");
        close(sock);
        cli->sockfd = -1;     // признак ошибки
        return;
    }
    setbuf(cli->stream, NULL); // отключаем буферизацию, чтобы fgets сразу читал из сокета
    cli->state = STATE_CMD;    // начальное состояние – ожидание команды
    cli->op = 0;
    cli->a = cli->b = 0;
    cli->filename[0] = '\0';
    cli->filesize = 0;
    cli->bytes_received = 0;
    cli->fp = NULL;
}

// Освобождение ресурсов клиента: закрываем файл и поток, затем сокет
void cleanup_client(client_t *cli) {
    if (cli->fp) {
        fclose(cli->fp);
        cli->fp = NULL;
    }
    if (cli->stream) {
        fclose(cli->stream); // это также закроет cli->sockfd
        cli->stream = NULL;
    }
    if (cli->sockfd >= 0) {
        close(cli->sockfd);
        cli->sockfd = -1;
    }
}

// Отправка строки клиенту (безопасная обёртка над write)
int send_msg(int sock, const char *msg) {
    if (!msg) return -1;
    size_t len = strlen(msg);
    ssize_t n = write(sock, msg, len);
    return (n == (ssize_t)len) ? 0 : -1;
}

// Чтение строки из потока с удалением символов конца строки
static int read_line(FILE *stream, char *buf, size_t size) {
    if (!fgets(buf, size, stream)) return 0;
    // удаляем \n и \r, если они есть
    char *nl = strchr(buf, '\n');
    if (nl) *nl = '\0';
    char *cr = strchr(buf, '\r');
    if (cr) *cr = '\0';
    return 1;
}

// Конечный автомат обработки команд клиента
int handle_client(client_t *cli) {
    char line[1024];
    int n;

    switch (cli->state) {
        case STATE_CMD:
            // Ждём строку с командой
            if (!read_line(cli->stream, line, sizeof(line)))
                return 0; // соединение закрыто или ошибка

            // Команда QUIT – прощаемся и отключаем
            if (strcasecmp(line, "QUIT") == 0) {
                send_msg(cli->sockfd, "Goodbye!\n");
                return 0;
            }

            // Команда FILE <имя_файла> – переходим к приёму файла
            if (strncasecmp(line, "FILE ", 5) == 0) {
                if (sscanf(line, "FILE %255s", cli->filename) == 1) {
                    send_msg(cli->sockfd, "READY\r\n");
                    cli->state = STATE_FILE_SIZE;
                } else {
                    send_msg(cli->sockfd, "ERROR: specify filename\r\n");
                    return 0;
                }
            }
            // Арифметические команды – запоминаем оператор и ждём первый параметр
            else if (strcasecmp(line, "ADD") == 0) {
                cli->op = '+';
                send_msg(cli->sockfd, "Enter 1 parameter\r\n");
                cli->state = STATE_PARAM1;
            }
            else if (strcasecmp(line, "SUB") == 0) {
                cli->op = '-';
                send_msg(cli->sockfd, "Enter 1 parameter\r\n");
                cli->state = STATE_PARAM1;
            }
            else if (strcasecmp(line, "MUL") == 0) {
                cli->op = '*';
                send_msg(cli->sockfd, "Enter 1 parameter\r\n");
                cli->state = STATE_PARAM1;
            }
            else if (strcasecmp(line, "DIV") == 0) {
                cli->op = '/';
                send_msg(cli->sockfd, "Enter 1 parameter\r\n");
                cli->state = STATE_PARAM1;
            }
            else {
                send_msg(cli->sockfd, "ERROR: unknown command\r\n");
                return 0;
            }
            return 1; // соединение остаётся активным

        case STATE_PARAM1:
            // Принимаем первый операнд
            if (!read_line(cli->stream, line, sizeof(line))) return 0;
            cli->a = atoi(line);
            send_msg(cli->sockfd, "Enter 2 parameter\r\n");
            cli->state = STATE_PARAM2;
            return 1;

        case STATE_PARAM2:
            // Принимаем второй операнд, вычисляем результат и завершаем сессию
            if (!read_line(cli->stream, line, sizeof(line))) return 0;
            cli->b = atoi(line);
            {
                int result;
                switch (cli->op) {
                    case '+': result = cli->a + cli->b; break;
                    case '-': result = cli->a - cli->b; break;
                    case '*': result = cli->a * cli->b; break;
                    case '/':
                        result = (cli->b != 0) ? cli->a / cli->b : 0;
                        break;
                    default: result = 0;
                }
                snprintf(line, sizeof(line), "%d\n", result);
                send_msg(cli->sockfd, line);
            }
            return 0; // завершаем сессию

        case STATE_FILE_SIZE:
            // Получаем размер файла (строка)
            if (!read_line(cli->stream, line, sizeof(line))) return 0;
            cli->filesize = atol(line);
            cli->fp = fopen(cli->filename, "wb");
            if (!cli->fp) {
                send_msg(cli->sockfd, "ERROR: cannot create file\r\n");
                return 0;
            }
            cli->bytes_received = 0;
            cli->state = STATE_FILE_DATA;
            // fallthrough – если размер пришёл вместе с началом данных, намеренно переходим к приёму данных

        case STATE_FILE_DATA:
            // Принимаем содержимое файла блоками по 8 Кбайт
            {
                char buf[8192];
                ssize_t to_read = sizeof(buf);
                if (cli->filesize - cli->bytes_received < to_read)
                    to_read = cli->filesize - cli->bytes_received;
                n = read(cli->sockfd, buf, to_read);
                if (n <= 0) {
                    fclose(cli->fp);
                    cli->fp = NULL;
                    return 0;
                }
                fwrite(buf, 1, n, cli->fp);
                cli->bytes_received += n;
                // если весь файл принят – сообщаем об успехе и завершаем
                if (cli->bytes_received >= cli->filesize) {
                    fclose(cli->fp);
                    cli->fp = NULL;
                    send_msg(cli->sockfd, "OK: file received\r\n");
                    printf("File '%s' received (%ld bytes)\n", cli->filename, cli->filesize);
                    return 0;
                }
            }
            return 1;

        default:
            return 0;
    }
}