#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "phonebook.h"

// Очистка буфера ввода
static void clear_input() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Вспомогательная функция: ввод строки с подстановкой старых значений
static void input_string_with_default(const char *prompt, char *dest, int size, const char *old_value) {
    char buffer[100];
    printf("%s [%s]: ", prompt, old_value);
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n')
            buffer[len-1] = '\0';
        if (buffer[0] != '\0')
            strcpy(dest, buffer);
    }
}

static Contact input_new_contact() {
    Contact c;
    c.id = 0; // временное значение
    char buffer[100];

    do {
        printf("Введите фамилию (обязательно): ");
        if (fgets(c.name.last_name, sizeof(c.name.last_name), stdin) != NULL) {
            size_t len = strlen(c.name.last_name);
            if (len > 0 && c.name.last_name[len-1] == '\n')
                c.name.last_name[len-1] = '\0';
            if (c.name.last_name[0] == '\0')
                printf("Фамилия обязательна.\n");
        }
    } while (c.name.last_name[0] == '\0');

    do {
        printf("Введите имя (обязательно): ");
        if (fgets(c.name.first_name, sizeof(c.name.first_name), stdin) != NULL) {
            size_t len = strlen(c.name.first_name);
            if (len > 0 && c.name.first_name[len-1] == '\n')
                c.name.first_name[len-1] = '\0';
            if (c.name.first_name[0] == '\0')
                printf("Имя обязательно.\n");
        }
    } while (c.name.first_name[0] == '\0');

    printf("Введите отчество (необязательно): ");
    if (fgets(c.name.middle_name, sizeof(c.name.middle_name), stdin) != NULL) {
        size_t len = strlen(c.name.middle_name);
        if (len > 0 && c.name.middle_name[len-1] == '\n')
            c.name.middle_name[len-1] = '\0';
    }

    printf("Введите место работы (необязательно): ");
    if (fgets(c.work.place, sizeof(c.work.place), stdin) != NULL) {
        size_t len = strlen(c.work.place);
        if (len > 0 && c.work.place[len-1] == '\n')
            c.work.place[len-1] = '\0';
    }

    printf("Введите должность (необязательно): ");
    if (fgets(c.work.position, sizeof(c.work.position), stdin) != NULL) {
        size_t len = strlen(c.work.position);
        if (len > 0 && c.work.position[len-1] == '\n')
            c.work.position[len-1] = '\0';
    }

    printf("Введите номер телефона (необязательно): ");
    if (fgets(c.phone, sizeof(c.phone), stdin) != NULL) {
        size_t len = strlen(c.phone);
        if (len > 0 && c.phone[len-1] == '\n')
            c.phone[len-1] = '\0';
    }

    printf("Введите email (необязательно): ");
    if (fgets(c.email, sizeof(c.email), stdin) != NULL) {
        size_t len = strlen(c.email);
        if (len > 0 && c.email[len-1] == '\n')
            c.email[len-1] = '\0';
    }

    printf("Введите ссылку на соцсеть (необязательно): ");
    if (fgets(c.social, sizeof(c.social), stdin) != NULL) {
        size_t len = strlen(c.social);
        if (len > 0 && c.social[len-1] == '\n')
            c.social[len-1] = '\0';
    }

    printf("Введите профиль в мессенджере (необязательно): ");
    if (fgets(c.messenger, sizeof(c.messenger), stdin) != NULL) {
        size_t len = strlen(c.messenger);
        if (len > 0 && c.messenger[len-1] == '\n')
            c.messenger[len-1] = '\0';
    }

    return c;
}

static void print_contact(const Contact *c) {
    printf("--- ID: %d ---\n", c->id);
    printf("Фамилия: %s\n", c->name.last_name);
    printf("Имя: %s\n", c->name.first_name);
    if (c->name.middle_name[0] != '\0')
        printf("Отчество: %s\n", c->name.middle_name);
    if (c->work.place[0] != '\0')
        printf("Место работы: %s\n", c->work.place);
    if (c->work.position[0] != '\0')
        printf("Должность: %s\n", c->work.position);
    if (c->phone[0] != '\0')
        printf("Телефон: %s\n", c->phone);
    if (c->email[0] != '\0')
        printf("Email: %s\n", c->email);
    if (c->social[0] != '\0')
        printf("Соцсеть: %s\n", c->social);
    if (c->messenger[0] != '\0')
        printf("Мессенджер: %s\n", c->messenger);
    printf("\n");
}

static void list_all_contacts() {
    Node *cur = get_head();
    if (cur == NULL) {
        printf("Телефонная книга пуста.\n");
        return;
    }
    printf("\nСписок всех контактов (в алфавитном порядке):\n");
    while (cur != NULL) {
        print_contact(&cur->data);
        cur = get_next(cur);
    }
}

static void print_menu() {
    printf("\n--- Телефонная книга (двусвязный список) ---\n");
    printf("1. Добавить контакт\n");
    printf("2. Редактировать контакт по ID\n");
    printf("3. Удалить контакт по ID\n");
    printf("4. Показать все контакты\n");
    printf("5. Поиск по фамилии/имени\n");
    printf("6. Поиск по ID\n");
    printf("0. Выход\n");
    printf("Выберите действие: ");
}

int main() {
    init_list();
    int choice;

    do {
        print_menu();
        if (scanf("%d", &choice) != 1) {
            printf("Ошибка ввода. Попробуйте снова.\n");
            clear_input();
            continue;
        }
        clear_input();

        switch (choice) {
            case 1: {
                Contact new_contact = input_new_contact();
                int new_id = add_contact(new_contact);
                if (new_id != -1)
                    printf("Контакт успешно добавлен. ID: %d\n", new_id);
                else
                    printf("Ошибка добавления (недостаточно памяти).\n");
                break;
            }

            case 2: {
                int id;
                printf("Введите ID контакта для редактирования: ");
                if (scanf("%d", &id) != 1) {
                    printf("Неверный ввод.\n");
                    clear_input();
                    break;
                }
                clear_input();

                const Contact *old = get_contact_by_id(id);
                if (old == NULL) {
                    printf("Контакта с ID %d не существует.\n", id);
                    break;
                }

                printf("Текущие данные контакта:\n");
                print_contact(old);
                printf("Введите новые данные (Enter - оставить без изменений).\n");

                Contact new_data = *old;

                input_string_with_default("Фамилия", new_data.name.last_name, sizeof(new_data.name.last_name), old->name.last_name);
                input_string_with_default("Имя", new_data.name.first_name, sizeof(new_data.name.first_name), old->name.first_name);
                input_string_with_default("Отчество", new_data.name.middle_name, sizeof(new_data.name.middle_name), old->name.middle_name);
                input_string_with_default("Место работы", new_data.work.place, sizeof(new_data.work.place), old->work.place);
                input_string_with_default("Должность", new_data.work.position, sizeof(new_data.work.position), old->work.position);
                input_string_with_default("Телефон", new_data.phone, sizeof(new_data.phone), old->phone);
                input_string_with_default("Email", new_data.email, sizeof(new_data.email), old->email);
                input_string_with_default("Соцсеть", new_data.social, sizeof(new_data.social), old->social);
                input_string_with_default("Мессенджер", new_data.messenger, sizeof(new_data.messenger), old->messenger);

                if (edit_contact(id, new_data))
                    printf("Контакт обновлён.\n");
                else
                    printf("Ошибка обновления.\n");
                break;
            }

            case 3: {
                int id;
                printf("Введите ID контакта для удаления: ");
                if (scanf("%d", &id) != 1) {
                    printf("Неверный ввод.\n");
                    clear_input();
                    break;
                }
                clear_input();

                if (delete_contact(id))
                    printf("Контакт удалён.\n");
                else
                    printf("Контакта с ID %d не существует.\n", id);
                break;
            }

            case 4:
                list_all_contacts();
                break;

            case 5: {
                char query[100];
                printf("Введите строку для поиска (фамилия или имя): ");
                if (fgets(query, sizeof(query), stdin) != NULL) {
                    size_t len = strlen(query);
                    if (len > 0 && query[len-1] == '\n')
                        query[len-1] = '\0';
                }
                int result_ids[100]; // массив для ID
                int found = search_by_name(query, result_ids);
                if (found == 0) {
                    printf("Ничего не найдено.\n");
                } else {
                    printf("Найдено контактов: %d\n", found);
                    for (int i = 0; i < found; i++) {
                        const Contact *c = get_contact_by_id(result_ids[i]);
                        if (c) {
                            printf("ID %d: %s %s %s\n", c->id,
                                   c->name.last_name, c->name.first_name,
                                   c->name.middle_name);
                        }
                    }
                }
                break;
            }

            case 6: {
                int id;
                printf("Введите ID контакта: ");
                if (scanf("%d", &id) != 1) {
                    printf("Неверный ввод.\n");
                    clear_input();
                    break;
                }
                clear_input();

                const Contact *c = get_contact_by_id(id);
                if (c == NULL)
                    printf("Контакта с ID %d не существует.\n", id);
                else
                    print_contact(c);
                break;
            }

            case 0:
                printf("Выход из программы.\n");
                free_list();
                break;

            default:
                printf("Неизвестная команда.\n");
        }
    } while (choice != 0);

    return 0;
}