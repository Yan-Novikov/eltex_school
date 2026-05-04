#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define SERVER_PORT 51000   // Порт для прослушки
#define BUFFER_SIZE 65536   // Максимальный размер IP-пакета

int main() {
    int raw_sock;
    unsigned char buffer[BUFFER_SIZE];
    ssize_t data_size;
    struct sockaddr_in saddr;
    socklen_t saddr_len = sizeof(saddr);
    FILE *dump_file = NULL;

    // cоздание RAW-сокета
    raw_sock = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (raw_sock < 0) {
        perror("socket(AF_INET, SOCK_RAW, IPPROTO_UDP)");
        fprintf(stderr, "Возможно, требуются права root (sudo)\n");
        exit(EXIT_FAILURE);
    }

    printf("RAW-сокет успешно создан. Ожидание UDP-пакетов на порт %d...\n", SERVER_PORT);
    printf("Для выхода нажмите Ctrl+C\n\n");

    // открытие файла для бинарного дампа
    dump_file = fopen("udp_dump.bin", "wb");
    if (!dump_file) {
        perror("fopen");
    }

    while (1) {
        // приём пакета
        data_size = recvfrom(raw_sock, buffer, BUFFER_SIZE, 0,
                             (struct sockaddr *)&saddr, &saddr_len);
        if (data_size < 0) {
            perror("recvfrom");
            break;
        }

        // разбор IP-заголовка
        struct ip *ip_header = (struct ip *)buffer;
        unsigned int ip_header_len = ip_header->ip_hl * 4;

        // проверка, что это UDP-пакет
        if (ip_header->ip_p != IPPROTO_UDP) {
            continue;
        }

        // разбор UDP-заголовка
        struct udphdr *udp_header = (struct udphdr *)(buffer + ip_header_len);

        // проверка порта назначения
        uint16_t dest_port = ntohs(udp_header->uh_dport);
        if (dest_port != SERVER_PORT) {
            continue;
        }

        // получение полезных данных
        unsigned char *payload = buffer + ip_header_len + sizeof(struct udphdr);
        ssize_t payload_len = data_size - (ip_header_len + sizeof(struct udphdr));

        // вывод информации о пакете
        char src_ip[INET_ADDRSTRLEN], dst_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(ip_header->ip_src), src_ip, INET_ADDRSTRLEN);
        inet_ntop(AF_INET, &(ip_header->ip_dst), dst_ip, INET_ADDRSTRLEN);

        printf("=== Захвачен UDP-пакет ===\n");
        printf("IP источника: %s:%d\n", src_ip, ntohs(udp_header->uh_sport));
        printf("IP назначения: %s:%d\n", dst_ip, ntohs(udp_header->uh_dport));
        printf("Длина UDP-дейтаграммы: %d байт\n", ntohs(udp_header->uh_ulen));
        printf("Контрольная сумма UDP: 0x%04x\n", ntohs(udp_header->uh_sum));
        printf("Данные (%zd байт): ", payload_len);

        if (payload_len > 0) {
            fwrite(payload, 1, payload_len, stdout);
            printf("\nHex-дамп первых 32-х байт: ");
            for (int i = 0; i < payload_len && i < 32; i++) {
                printf("%02x ", payload[i]);
            }
            printf("\n");
        } else {
            printf("нет данных\n");
        }
        printf("===========================\n\n");
        fflush(stdout);

        // запись в дамп-файл
        if (dump_file) {
            fwrite(buffer, 1, data_size, dump_file);
            fflush(dump_file);
        }
    }

    if (dump_file) {
        fclose(dump_file);
        printf("Бинарный дамп сохранён в udp_dump.bin\n");
    }

    close(raw_sock);
    return 0;
}