#include "signal_handler.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int file_fd = -1;
int sigint_flag = 0;
int sigint_count = 0;
int sigquit_flag = 0;

// SIGINT и SIGQUIT
void signal_handler(int sig) {
    char buf[64];
    int len;

    if (sig == SIGINT) {
        sigint_count++;
        sigint_flag++;
    } else if (sig == SIGQUIT) {
        sigquit_flag++;
    } else {
        return;
    }
}

void setup_handlers() {
    signal(SIGINT, signal_handler);
    signal(SIGQUIT, signal_handler);
}