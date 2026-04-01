#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "permission.h"

static void print_permissions(mode_t mode) {
    char symbolic[10];
    char numeric[4];
    char binary[10];

    format_symbolic(mode, symbolic, sizeof(symbolic));
    format_numeric(mode, numeric, sizeof(numeric));
    format_binary(mode, binary, sizeof(binary));

    printf("Буквенное: %s\n", symbolic);
    printf("Цифровое: %s\n", numeric);
    printf("Битовое: %s\n", binary);
}

int main() {
    mode_t current_mode = 0;
    int mode_set = 0;
    char input[256];

    while (1) {
        printf("\nМеню:\n");
        printf("1. Ввести права доступа (буквенно или цифрами) и показать битовое представление\n");
        printf("2. Ввести имя файла и показать права доступа (буквенно, цифрами, битово)\n");
        printf("3. Изменить права доступа (из предыдущего пункта) с помощью модификатора (например, u+x)\n");
        printf("4. Выход\n");
        printf("Выберите пункт: ");

        if (!fgets(input, sizeof(input), stdin))
            break;
        int choice = atoi(input);

        switch (choice) {
            case 1: {
                printf("Введите права доступа (например, 755 или rwxr-xr--): ");
                if (!fgets(input, sizeof(input), stdin))
                    break;
                input[strcspn(input, "\n")] = '\0';

                mode_t mode;
                if (parse_permission_string(input, &mode) == 0) {
                    current_mode = mode;
                    mode_set = 1;
                    char binary[10];
                    format_binary(mode, binary, sizeof(binary));
                    printf("Битовое представление: %s\n", binary);
                    // Дополнительно буквенное и цифровое
                    char symbolic[10], numeric[4];
                    format_symbolic(mode, symbolic, sizeof(symbolic));
                    format_numeric(mode, numeric, sizeof(numeric));
                    printf("Буквенное: %s\n", symbolic);
                    printf("Цифровое: %s\n", numeric);
                } else {
                    printf("Ошибка: неверный формат прав доступа.\n");
                }
                break;
            }
            case 2: {
                printf("Введите имя файла: ");
                if (!fgets(input, sizeof(input), stdin))
                    break;
                input[strcspn(input, "\n")] = '\0';

                struct stat st;
                if (stat(input, &st) == 0) {
                    mode_t file_mode = st.st_mode & 0777;
                    current_mode = file_mode;
                    mode_set = 1;

                    printf("Права доступа к файлу %s:\n", input);
                    print_permissions(file_mode);

                    // Формируем строку, аналогичную ls -l
                    char type_char = '?';
                    if (S_ISREG(st.st_mode)) type_char = '-';
                    else if (S_ISDIR(st.st_mode)) type_char = 'd';
                    else if (S_ISLNK(st.st_mode)) type_char = 'l';
                    else if (S_ISCHR(st.st_mode)) type_char = 'c';
                    else if (S_ISBLK(st.st_mode)) type_char = 'b';
                    else if (S_ISFIFO(st.st_mode)) type_char = 'p';
                    else if (S_ISSOCK(st.st_mode)) type_char = 's';

                    char symbolic[10];
                    format_symbolic(file_mode, symbolic, sizeof(symbolic));
                    printf("Сравните с выводом ls -l: %c%s %s\n", type_char, symbolic, input);
                } else {
                    perror("stat");
                }
                break;
            }
            case 3: {
                if (!mode_set) {
                    printf("Сначала введите права доступа (пункт 1) или получите из файла (пункт 2).\n");
                    break;
                }
                printf("Текущие права:\n");
                print_permissions(current_mode);
                printf("Введите модификатор (например, u+x, g-w, o=r, ug+rw, a-x): ");
                if (!fgets(input, sizeof(input), stdin))
                    break;
                input[strcspn(input, "\n")] = '\0';

                mode_t new_mode;
                if (apply_modifier(current_mode, input, &new_mode) == 0) {
                    printf("Новые права после применения модификатора '%s':\n", input);
                    print_permissions(new_mode);
                    // Обновляем текущие права для последовательных изменений
                    current_mode = new_mode;
                } else {
                    printf("Ошибка: неверный модификатор.\n");
                }
                break;
            }
            case 4:
                printf("Выход.\n");
                return 0;
            default:
                printf("Неверный выбор. Попробуйте снова.\n");
                break;
        }
    }
    return 0;
}