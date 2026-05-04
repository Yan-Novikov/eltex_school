#ifndef SIGNAL_HANDLERS_H
#define SIGNAL_HANDLERS_H

#include <signal.h>

// Инициализация обработчиков, fd - дескриптор файла для записи
void setup_handlers(void);

// Закрытие файла (вызывается при завершении)
void cleanup(void);

extern int file_fd;
extern int sigint_flag;
extern int sigint_count;
extern int sigquit_flag;

#endif