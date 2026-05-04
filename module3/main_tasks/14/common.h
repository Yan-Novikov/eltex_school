#ifndef COMMON_H
#define COMMON_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define SERVER_PORT     51000
#define MAX_MSG_LEN     1000
#define MAX_CLIENTS     2

int create_udp_socket();
void fill_addr(struct sockaddr_in *addr, const char *ip, int port);

#endif