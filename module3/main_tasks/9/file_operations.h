#ifndef FILE_OPERATIONS_H
#define FILE_OPERATIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define DATA_FILE "data.txt"

void write_random_line(const char *filename);
void process_and_clear_file(const char *filename);

#endif