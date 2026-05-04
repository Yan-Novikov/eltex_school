#include "client.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <номер>\n", argv[0]);
        exit(1);
    }
    int id = atoi(argv[1]);
    if (id < 20) {
        fprintf(stderr, "Номер должен быть >= 20\n");
        exit(1);
    }
    key_t key = ftok(KEY_PATH, 'A');
    if (key == -1) {
        perror("ftok");
        exit(1);
    }
    run_client(get_queue(key, 0), id);
    return 0;
}