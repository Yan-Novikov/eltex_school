#include "permission.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

static int parse_numeric(const char *str, mode_t *mode) {
    if (strlen(str) != 3) return -1;
    for (int i = 0; i < 3; i++) {
        if (!isdigit(str[i]) || str[i] < '0' || str[i] > '7')
            return -1;
    }

    int user = str[0] - '0';
    int group = str[1] - '0';
    int other = str[2] - '0';
    mode_t m = 0;

    // u
    if (user & 4) m |= S_IRUSR;
    if (user & 2) m |= S_IWUSR;
    if (user & 1) m |= S_IXUSR;
    // g
    if (group & 4) m |= S_IRGRP;
    if (group & 2) m |= S_IWGRP;
    if (group & 1) m |= S_IXGRP;
    // o
    if (other & 4) m |= S_IROTH;
    if (other & 2) m |= S_IWOTH;
    if (other & 1) m |= S_IXOTH;

    *mode = m;
    return 0;
}

static int parse_symbolic(const char *str, mode_t *mode) {
    if (strlen(str) != 9) return -1;
    for (int i = 0; i < 9; i++) {
        if (str[i] != 'r' && str[i] != 'w' && str[i] != 'x' && str[i] != '-')
            return -1;
    }

    mode_t m = 0;
    if (str[0] == 'r') m |= S_IRUSR;
    if (str[1] == 'w') m |= S_IWUSR;
    if (str[2] == 'x') m |= S_IXUSR;
    if (str[3] == 'r') m |= S_IRGRP;
    if (str[4] == 'w') m |= S_IWGRP;
    if (str[5] == 'x') m |= S_IXGRP;
    if (str[6] == 'r') m |= S_IROTH;
    if (str[7] == 'w') m |= S_IWOTH;
    if (str[8] == 'x') m |= S_IXOTH;

    *mode = m;
    return 0;
}

int parse_permission_string(const char *str, mode_t *mode) {
    if (!str) return -1;
    size_t len = strlen(str);

    // Попытка распознать числовой формат
    if (len == 3) {
        int numeric = 1;
        for (size_t i = 0; i < len; i++) {
            if (!isdigit(str[i])) {
                numeric = 0;
                break;
            }
        }
        if (numeric) return parse_numeric(str, mode);
    }

    // Попытка распознать символьный формат
    if (len == 9) {
        int symbolic = 1;
        for (size_t i = 0; i < len; i++) {
            if (str[i] != 'r' && str[i] != 'w' && str[i] != 'x' && str[i] != '-') {
                symbolic = 0;
                break;
            }
        }
        if (symbolic) return parse_symbolic(str, mode);
    }

    return -1;
}

void format_symbolic(mode_t mode, char *buf, size_t size) {
    if (size < 10) {
        if (size > 0) buf[0] = '\0';
        return;
    }
    buf[0] = (mode & S_IRUSR) ? 'r' : '-';
    buf[1] = (mode & S_IWUSR) ? 'w' : '-';
    buf[2] = (mode & S_IXUSR) ? 'x' : '-';
    buf[3] = (mode & S_IRGRP) ? 'r' : '-';
    buf[4] = (mode & S_IWGRP) ? 'w' : '-';
    buf[5] = (mode & S_IXGRP) ? 'x' : '-';
    buf[6] = (mode & S_IROTH) ? 'r' : '-';
    buf[7] = (mode & S_IWOTH) ? 'w' : '-';
    buf[8] = (mode & S_IXOTH) ? 'x' : '-';
    buf[9] = '\0';
}

void format_numeric(mode_t mode, char *buf, size_t size) {
    snprintf(buf, size, "%03o", mode & 0777);
}

void format_binary(mode_t mode, char *buf, size_t size) {
    if (size < 10) {
        if (size > 0) buf[0] = '\0';
        return;
    }
    buf[0] = (mode & S_IRUSR) ? '1' : '0';
    buf[1] = (mode & S_IWUSR) ? '1' : '0';
    buf[2] = (mode & S_IXUSR) ? '1' : '0';
    buf[3] = (mode & S_IRGRP) ? '1' : '0';
    buf[4] = (mode & S_IWGRP) ? '1' : '0';
    buf[5] = (mode & S_IXGRP) ? '1' : '0';
    buf[6] = (mode & S_IROTH) ? '1' : '0';
    buf[7] = (mode & S_IWOTH) ? '1' : '0';
    buf[8] = (mode & S_IXOTH) ? '1' : '0';
    buf[9] = '\0';
}

static int apply_one_modifier(mode_t *mode, const char *who_str, char op, const char *perms_str) {
    // Проверка (только u,g,o,a)
    for (const char *p = who_str; *p; p++) {
        if (*p != 'u' && *p != 'g' && *p != 'o' && *p != 'a')
            return -1;
    }

    // Для + и - perms_str не может быть пустым
    if ((op == '+' || op == '-') && (perms_str == NULL || *perms_str == '\0'))
        return -1;

    // Проверка (только r,w,x)
    if (perms_str != NULL) {
        for (const char *p = perms_str; *p; p++) {
            if (*p != 'r' && *p != 'w' && *p != 'x')
                return -1;
        }
    }

    // Применяем изменения для каждого who
    for (const char *w = who_str; *w; w++) {
        char who = *w;
        mode_t mask_r = 0, mask_w = 0, mask_x = 0;

        switch (who) {
            case 'u':
                mask_r = S_IRUSR; 
                mask_w = S_IWUSR; 
                mask_x = S_IXUSR;
                break;
            case 'g':
                mask_r = S_IRGRP; 
                mask_w = S_IWGRP; 
                mask_x = S_IXGRP;
                break;
            case 'o':
                mask_r = S_IROTH; 
                mask_w = S_IWOTH; 
                mask_x = S_IXOTH;
                break;
            case 'a':
                mask_r = S_IRUSR | S_IRGRP | S_IROTH;
                mask_w = S_IWUSR | S_IWGRP | S_IWOTH;
                mask_x = S_IXUSR | S_IXGRP | S_IXOTH;
                break;
            default:
                return -1;
        }

        // Для операции '=' сначала сбрасываем все биты этой категории
        if (op == '=') {
            if (who == 'a') {
                *mode &= ~(S_IRWXU | S_IRWXG | S_IRWXO);
            } else {
                mode_t clear_mask = 0;
                switch (who) {
                    case 'u': clear_mask = S_IRWXU; break;
                    case 'g': clear_mask = S_IRWXG; break;
                    case 'o': clear_mask = S_IRWXO; break;
                    default: break;
                }
                *mode &= ~clear_mask;
            }
        }

        // Устанавливаем или сбрасываем биты в соответствии с perms_str
        if (perms_str != NULL) {
            for (const char *p = perms_str; *p; p++) {
                mode_t bit = 0;
                switch (*p) {
                    case 'r': bit = mask_r; break;
                    case 'w': bit = mask_w; break;
                    case 'x': bit = mask_x; break;
                    default: return -1;
                }
                if (op == '+' || op == '=')
                    *mode |= bit;
                else if (op == '-')
                    *mode &= ~bit;
            }
        }
    }
    return 0;
}

int apply_modifier(mode_t current, const char *modifier, mode_t *new_mode) {
    if (!modifier) return -1;

    mode_t mode = current;
    char mod_copy[256];
    strncpy(mod_copy, modifier, sizeof(mod_copy) - 1);
    mod_copy[sizeof(mod_copy) - 1] = '\0';

    char *token = strtok(mod_copy, ",");
    while (token) {
        // Находим символ операции (+, -, =)
        char *op_pos = NULL;
        for (char *p = token; *p; p++) {
            if (*p == '+' || *p == '-' || *p == '=') {
                op_pos = p;
                break;
            }
        }
        if (!op_pos) return -1;

        char op = *op_pos;
        size_t who_len = op_pos - token;
        char who_str[4]; // максимум "ugo" + нуль
        if (who_len >= sizeof(who_str)) return -1;

        if (who_len > 0) {
            strncpy(who_str, token, who_len);
            who_str[who_len] = '\0';
        } else {
            who_str[0] = 'a';
            who_str[1] = '\0';
        }

        const char *perms_str = op_pos + 1; // может быть пустым

        if (apply_one_modifier(&mode, who_str, op, perms_str) != 0)
            return -1;

        token = strtok(NULL, ",");
    }

    *new_mode = mode;
    return 0;
}