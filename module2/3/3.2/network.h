#ifndef NETWORK_H
#define NETWORK_H

#include <stdint.h>

uint32_t ip_to_uint32(const char *ip);

int is_in_subnet(uint32_t ip, uint32_t gateway, uint32_t mask);

#endif