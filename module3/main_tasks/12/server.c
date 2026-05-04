#include "common.h"

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

int main() {
    int sockfd;
    struct sockaddr_in serv_addr, client_addrs[MAX_CLIENTS];
    int client_count = 0;
    char buffer[MAX_MSG_LEN];
    socklen_t addr_len;
    int n;

    sockfd = create_udp_socket();
    fill_addr(&serv_addr, NULL, SERVER_PORT);

    if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind");
        close(sockfd);
        exit(1);
    }

    printf("UDP Chat Server started on port %d\n", SERVER_PORT);
    printf("Waiting for clients (max %d)...\n", MAX_CLIENTS);

    memset(client_addrs, 0, sizeof(client_addrs));

    while (1) {
        struct sockaddr_in sender_addr;
        addr_len = sizeof(sender_addr);

        n = recvfrom(sockfd, buffer, MAX_MSG_LEN - 1, 0,
                     (struct sockaddr*)&sender_addr, &addr_len);
        if (n < 0) {
            perror("recvfrom");
            continue;
        }
        buffer[n] = '\0';

        size_t len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n')
            buffer[len-1] = '\0';

        // Ищем отправителя среди зарегистрированных
        int sender_idx = -1;
        for (int i = 0; i < client_count; i++) {
            if (client_addrs[i].sin_addr.s_addr == sender_addr.sin_addr.s_addr &&
                client_addrs[i].sin_port == sender_addr.sin_port) {
                sender_idx = i;
                break;
            }
        }

        // Если отправитель новый и есть свободные места — регистрируем
        if (sender_idx == -1) {
            if (client_count < MAX_CLIENTS) {
                client_addrs[client_count] = sender_addr;
                sender_idx = client_count;
                client_count++;
                printf("Client %d connected: %s:%d\n",
                       sender_idx + 1,
                       inet_ntoa(sender_addr.sin_addr),
                       ntohs(sender_addr.sin_port));

                // Если теперь в чате двое — уведомляем обоих
                if (client_count == 2) {
                    const char *msg1 = "Your chat partner has joined. You can start chatting.\n";
                    sendto(sockfd, msg1, strlen(msg1), 0,
                           (struct sockaddr*)&client_addrs[0], sizeof(client_addrs[0]));
                    const char *msg2 = "You have joined the chat. Say hello!\n";
                    sendto(sockfd, msg2, strlen(msg2), 0,
                           (struct sockaddr*)&client_addrs[1], sizeof(client_addrs[1]));
                }
            } else {
                const char *full_msg = "Chat room is full. Try later.\n";
                sendto(sockfd, full_msg, strlen(full_msg), 0,
                       (struct sockaddr*)&sender_addr, addr_len);
                continue;
            }
        }

        // Обработка выхода
        if (strcmp(buffer, "exit") == 0) {
            printf("Client %d (%s:%d) disconnected\n",
                   sender_idx + 1,
                   inet_ntoa(sender_addr.sin_addr),
                   ntohs(sender_addr.sin_port));

            // Уведомляем оставшегося клиента
            if (client_count == 2) {
                int other_idx = (sender_idx == 0) ? 1 : 0;
                const char *bye_msg = "Your chat partner has left the chat.\n";
                sendto(sockfd, bye_msg, strlen(bye_msg), 0,
                       (struct sockaddr*)&client_addrs[other_idx],
                       sizeof(client_addrs[other_idx]));
            }

            // Удаляем отключившегося из массива
            for (int i = sender_idx; i < client_count - 1; i++) {
                client_addrs[i] = client_addrs[i + 1];
            }
            client_count--;
            continue;
        }

        // Если в чате двое — пересылаем сообщение другому
        if (client_count == 2) {
            int target_idx = (sender_idx == 0) ? 1 : 0;
            n = sendto(sockfd, buffer, strlen(buffer), 0,
                       (struct sockaddr*)&client_addrs[target_idx],
                       sizeof(client_addrs[target_idx]));
            if (n < 0) {
                perror("sendto");
            } else {
                printf("Forwarded message from client %d to client %d\n",
                       sender_idx + 1, target_idx + 1);
            }
        } else {
            // Если клиент один — сообщаем, что ждём второго
            const char *wait_msg = "Waiting for another client to join...\n";
            sendto(sockfd, wait_msg, strlen(wait_msg), 0,
                   (struct sockaddr*)&sender_addr, addr_len);
        }
    }

    close(sockfd);
    return 0;
}