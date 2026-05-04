#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Нет аргументов.\n");
        return 0;
    }
    int max_len = 0;
    char *max_str = NULL;
    for (int i = 1; i < argc; i++) {
        int len = strlen(argv[i]);
        if (len > max_len) {
            max_len = len;
            max_str = argv[i];
        }
    }
    printf("Строка с максимальной длиной (%d): %s\n", max_len, max_str);
    return 0;
}