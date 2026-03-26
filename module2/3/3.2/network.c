#include "network.h"
#include <stdlib.h>
#include <string.h>

uint32_t ip_to_uint32(const char *ip) {
    uint32_t val = 0;
    char *ip_copy = strdup(ip);
    char *token = strtok(ip_copy, ".");

    for (int i = 0; i < 4; ++i) {
        if (token != NULL) {
            uint8_t octet = (uint8_t)atoi(token);
            val = (val << 8) | octet;
            token = strtok(NULL, ".");
        }
    }

    free(ip_copy);
    return val;
}

int is_in_subnet(uint32_t ip, uint32_t gateway, uint32_t mask) {
    return (ip & mask) == (gateway & mask);
}