#ifndef COMMON_H
#define COMMON_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define SERVER_TYPE 10
#define TEXT_SIZE   256
#define KEY_PATH    "./chat_server"

struct chat_msg {
    long mtype;
    int  sender_id;
    char text[TEXT_SIZE];
};

int get_queue(key_t key, int create);

#endif