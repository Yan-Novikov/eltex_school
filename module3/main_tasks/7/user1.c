#include "chat.h"
#include <stdio.h>
#include <string.h>

int main() {
    mqd_t send_mq, recv_mq;
    if (chat_init(1, &send_mq, &recv_mq) != 0)
        return 1;

    printf("Чат User1. Вы начинаете. 'exit' для выхода.\n");

    char buffer[MSG_SIZE];
    unsigned prio;

    while (1) {
        printf("Вы: ");
        fflush(stdout);
        if (!fgets(buffer, MSG_SIZE, stdin)) break;
        buffer[strcspn(buffer, "\n")] = '\0';

        if (chat_send(send_mq, buffer) != 0) break;
        if (strcmp(buffer, "exit") == 0) break;

        if (chat_receive(recv_mq, buffer, MSG_SIZE, &prio) < 0) break;
        if (prio == PRIO_EXIT) {
            printf("Собеседник завершил чат.\n");
            break;
        }
        printf("User2: %s\n", buffer);
    }

    chat_cleanup(send_mq, recv_mq);
    return 0;
}