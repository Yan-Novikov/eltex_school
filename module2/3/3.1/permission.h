#ifndef PERMISSION_H
#define PERMISSION_H

#include <sys/types.h>
#include <sys/stat.h>

int parse_permission_string(const char *str, mode_t *mode);

void format_symbolic(mode_t mode, char *buf, size_t size);

void format_numeric(mode_t mode, char *buf, size_t size);

void format_binary(mode_t mode, char *buf, size_t size);

int apply_modifier(mode_t current, const char *modifier, mode_t *new_mode);

#endif