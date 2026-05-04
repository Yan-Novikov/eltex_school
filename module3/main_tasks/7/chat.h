#ifndef CHAT_H
#define CHAT_H

#include <mqueue.h>
#include <stddef.h>

#define MSG_SIZE     256
#define MAX_MSGS     10
#define PRIO_NORMAL  1
#define PRIO_EXIT    99

#define QUEUE_NAME_1 "/chat_1_to_2"
#define QUEUE_NAME_2 "/chat_2_to_1"

// Инициализация очередей для заданной роли
int chat_init(int role, mqd_t *send_mq, mqd_t *recv_mq);

// Закрытие и удаление очередей
void chat_cleanup(mqd_t send_mq, mqd_t recv_mq);

int chat_send(mqd_t mq, const char *msg);

ssize_t chat_receive(mqd_t mq, char *buf, size_t size, unsigned *prio);

#endif