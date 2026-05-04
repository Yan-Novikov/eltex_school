#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

// Структура для хранения информации об одной команде
typedef struct {
    char *argv[MAX_ARGS];   // аргументы команды
    char *input_file;       // файл для перенаправления ввода
    char *output_file;      // файл для перенаправления вывода
} Command;

static void parse_command_with_redir(char *cmd_str, Command *cmd) {
    cmd->input_file = NULL;
    cmd->output_file = NULL;
    int argc = 0;
    char *saveptr;
    char *token = strtok_r(cmd_str, " \t", &saveptr);
    while (token != NULL && argc < MAX_ARGS - 1) {
        if (strcmp(token, "<") == 0) {
            token = strtok_r(NULL, " \t", &saveptr);
            if (token != NULL) cmd->input_file = token;
        } else if (strcmp(token, ">") == 0) {
            token = strtok_r(NULL, " \t", &saveptr);
            if (token != NULL) cmd->output_file = token;
        } else {
            cmd->argv[argc++] = token;
        }
        token = strtok_r(NULL, " \t", &saveptr);
    }
    cmd->argv[argc] = NULL;
}

int execute_pipeline(char *cmdline) {
    if (cmdline == NULL || strlen(cmdline) == 0)
        return 0;
    
    // Разбиваем строку на отдельные команды
    char *commands[MAX_CMDS];
    int num_cmds = 0;
    char *saveptr;
    char *cmd_token = strtok_r(cmdline, "|", &saveptr);
    while (cmd_token != NULL && num_cmds < MAX_ARGS) {
        // Удаляем пробелы в начале и конце
        while (*cmd_token == ' ' || *cmd_token == '\t') cmd_token++;
        char *end = cmd_token + strlen(cmd_token) - 1;
        while (end > cmd_token && (*end == ' ' || *end == '\t')) *end-- = '\0';
        commands[num_cmds++] = cmd_token;
        cmd_token = strtok_r(NULL, "|", &saveptr);
    }
    
    if (num_cmds == 0) return 0;
    
    // Парсим каждую команду
    Command cmd_list[MAX_CMDS];
    for (int i = 0; i < num_cmds; i++) {
        parse_command_with_redir(commands[i], &cmd_list[i]);
        if (cmd_list[i].argv[0] == NULL) {
            fprintf(stderr, "Ошибка: пустая команда\n");
            return -1;
        }
        // Перенаправление вывода допустимо только для последней команды
        if (i != num_cmds - 1 && cmd_list[i].output_file != NULL) {
            fprintf(stderr, "Предупреждение: '>' разрешён только в последней команде, игнорируется\n");
            cmd_list[i].output_file = NULL;
        }
    }
    
    // Создание pipe для каждой пары команд
    int pipes[2 * (num_cmds - 1)]; // каждые два элемента – один pipe
    for (int i = 0; i < num_cmds - 1; i++) {
        if (pipe(pipes + 2 * i) == -1) {
            perror("pipe");
            return -1;
        }
    }
    
    // Создание дочерних процессов (fork)
    pid_t pids[MAX_CMDS];
    for (int i = 0; i < num_cmds; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            return -1;
        }
        if (pid == 0) { // дочерний процесс
            // Перенаправление ввода
            if (cmd_list[i].input_file != NULL) {
                int fd_in = open(cmd_list[i].input_file, O_RDONLY);
                if (fd_in == -1) {
                    perror("open input file");
                    exit(1);
                }
                dup2(fd_in, STDIN_FILENO);
                close(fd_in);
            } else if (i > 0) {
                // Взять чтение из предыдущего pipe
                dup2(pipes[2*(i-1) + 0], STDIN_FILENO);
            }
            
            // Перенаправление вывода
            if (cmd_list[i].output_file != NULL) {
                int fd_out = open(cmd_list[i].output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd_out == -1) {
                    perror("open output file");
                    exit(1);
                }
                dup2(fd_out, STDOUT_FILENO);
                close(fd_out);
            } else if (i < num_cmds - 1) {
                // Запись в следующий pipe
                dup2(pipes[2*i + 1], STDOUT_FILENO);
            }
            
            // Закрыть все дескрипторы pipe в дочернем процессе
            for (int j = 0; j < num_cmds - 1; j++) {
                close(pipes[2*j + 0]);
                close(pipes[2*j + 1]);
            }
            
            // Запуск программы (exec)
            execvp(cmd_list[i].argv[0], cmd_list[i].argv);
            perror("execvp");
            exit(1);
        } else {
            pids[i] = pid;
        }
    }
    
    // Родительский процесс (закрыть все концы pipe)
    for (int i = 0; i < num_cmds - 1; i++) {
        close(pipes[2*i + 0]);
        close(pipes[2*i + 1]);
    }
    
    // Ждём завершения всех дочерних процессов
    for (int i = 0; i < num_cmds; i++) {
        waitpid(pids[i], NULL, 0);
    }
    
    return 0;
}

// Функция чтения команды (из второго задания)
char* read_command() {
    static char cmdline[MAX_CMD_LEN];
    printf("> ");
    fflush(stdout);
    if (fgets(cmdline, sizeof(cmdline), stdin) == NULL) {
        return NULL;
    }
    size_t len = strlen(cmdline);
    if (len > 0 && cmdline[len-1] == '\n')
        cmdline[len-1] = '\0';
    return cmdline;
}