#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "network.h"

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <gateway_ip> <subnet_mask> <N>\n", argv[0]);
        return -1;
    }

    const char *gateway_ip_str = argv[1];
    const char *subnet_mask_str = argv[2];
    int N = atoi(argv[3]);

    if (N <= 0) {
        fprintf(stderr, "N must be a positive integer.\n");
        return -1;
    }

    uint32_t gateway = ip_to_uint32(gateway_ip_str);
    uint32_t mask = ip_to_uint32(subnet_mask_str);

    srand(time(NULL));

    int same_subnet_count = 0;
    int other_subnet_count = 0;

    // Генерация N случайных IP-адресов и проверка принадлежности подсети
    for (int i = 0; i < N; ++i) {
        uint32_t random_ip = 0;
        for (int j = 0; j < 4; ++j) {
            random_ip = (random_ip << 8) | (rand() & 0xFF);
        }

        if (is_in_subnet(random_ip, gateway, mask)) {
            same_subnet_count++;
        } else {
            other_subnet_count++;
        }
    }

    double same_percent = (double)same_subnet_count / N * 100.0;
    double other_percent = (double)other_subnet_count / N * 100.0;

    printf("Statistics:\n");
    printf("Packets for own subnet: %d (%.2f%%)\n", same_subnet_count, same_percent);
    printf("Packets for other networks: %d (%.2f%%)\n", other_subnet_count, other_percent);

    return 0;
}