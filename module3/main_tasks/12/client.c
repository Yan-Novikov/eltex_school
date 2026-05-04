#include "common.h"
#include <signal.h>
#include <sys/wait.h>

int create_udp_socket() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }
    return sock;
}

void fill_addr(struct sockaddr_in *addr, const char *ip, int port) {
    memset(addr, 0, sizeof(*addr));
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    if (ip == NULL) {
        addr->sin_addr.s_addr = htonl(INADDR_ANY);
    } else {
        if (inet_aton(ip, &addr->sin_addr) == 0) {
            fprintf(stderr, "Invalid IP address: %s\n", ip);
            exit(1);
        }
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <server IP>\n", argv[0]);
        exit(1);
    }

    int sockfd;
    struct sockaddr_in serv_addr, my_addr;
    char buffer[MAX_MSG_LEN];

    sockfd = create_udp_socket();

    fill_addr(&my_addr, NULL, 0);
    if (bind(sockfd, (struct sockaddr*)&my_addr, sizeof(my_addr)) < 0) {
        perror("bind");
        close(sockfd);
        exit(1);
    }

    fill_addr(&serv_addr, argv[1], SERVER_PORT);

    printf("UDP Chat Client. Type messages and press Enter.\n");
    printf("Type 'exit' to quit.\n");
    printf("----------------------------------------\n");

    // Отправляем пустое сообщение, чтобы сервер зарегистрировал клиента сразу
    const char *join_msg = "";
    sendto(sockfd, join_msg, 1, 0,  // отправляем '\0'
           (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        close(sockfd);
        exit(1);
    }

    if (pid == 0) { // Дочерний
        struct sockaddr_in from_addr;
        socklen_t from_len = sizeof(from_addr);
        while (1) {
            int n = recvfrom(sockfd, buffer, MAX_MSG_LEN - 1, 0,
                             (struct sockaddr*)&from_addr, &from_len);
            if (n < 0) {
                perror("recvfrom");
                break;
            }
            buffer[n] = '\0';

            // Если пришла пустая строка, игнорируем
            if (strlen(buffer) == 0)
                continue;

            printf("\n[Friend]: %s\n", buffer);
            printf("You: ");
            fflush(stdout);
        }
        _exit(0);
    } else { // Родительский
        while (1) {
            printf("You: ");
            fflush(stdout);

            if (fgets(buffer, MAX_MSG_LEN, stdin) == NULL) {
                break;
            }

            size_t len = strlen(buffer);
            if (len > 0 && buffer[len-1] == '\n')
                buffer[len-1] = '\0';

            if (strlen(buffer) == 0)
                continue;

            int n = sendto(sockfd, buffer, strlen(buffer), 0,
                           (struct sockaddr*)&serv_addr, sizeof(serv_addr));
            if (n < 0) {
                perror("sendto");
                break;
            }

            if (strcmp(buffer, "exit") == 0) {
                printf("Disconnecting...\n");
                break;
            }
        }

        kill(pid, SIGTERM);
        wait(NULL);
        close(sockfd);
    }

    return 0;
}