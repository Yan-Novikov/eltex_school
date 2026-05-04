#include "chat.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

int chat_init(int role, mqd_t *send_mq, mqd_t *recv_mq) {
    const char *send_name, *recv_name;
    if (role == 1) {
        send_name = QUEUE_NAME_1;
        recv_name = QUEUE_NAME_2;
    } else {
        send_name = QUEUE_NAME_2;
        recv_name = QUEUE_NAME_1;
    }

    struct mq_attr attr = {
        .mq_flags   = 0,
        .mq_maxmsg  = MAX_MSGS,
        .mq_msgsize = MSG_SIZE,
        .mq_curmsgs = 0
    };

    *send_mq = mq_open(send_name, O_CREAT | O_RDWR, 0600, &attr);
    if (*send_mq == (mqd_t)-1) {
        perror("mq_open send");
        return -1;
    }

    *recv_mq = mq_open(recv_name, O_CREAT | O_RDWR, 0600, &attr);
    if (*recv_mq == (mqd_t)-1) {
        perror("mq_open recv");
        mq_close(*send_mq);
        mq_unlink(send_name);
        return -1;
    }

    return 0;
}

void chat_cleanup(mqd_t send_mq, mqd_t recv_mq) {
    mq_close(send_mq);
    mq_close(recv_mq);
    mq_unlink(QUEUE_NAME_1);
    mq_unlink(QUEUE_NAME_2);
}

int chat_send(mqd_t mq, const char *msg) {
    unsigned prio = PRIO_NORMAL;
    size_t len = strlen(msg);
    if (strcmp(msg, "exit") == 0) {
        prio = PRIO_EXIT;
        len = 4; // отправляем только "exit"
    }
    if (mq_send(mq, msg, len, prio) == -1) {
        perror("mq_send");
        return -1;
    }
    return 0;
}

ssize_t chat_receive(mqd_t mq, char *buf, size_t size, unsigned *prio) {
    ssize_t n = mq_receive(mq, buf, size, prio);
    if (n == -1) {
        perror("mq_receive");
        return -1;
    }
    buf[n] = '\0';
    return n;
}