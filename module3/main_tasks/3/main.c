#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "phonebook.h"

#define MAX 10
#define DATA_FILE "phonebook.dat"

static void read_str(char *dest, int size, int mandatory) {
    char buf[100];
    do {
        fgets(buf, sizeof(buf), stdin);
        size_t len = strlen(buf);
        if (len && buf[len-1] == '\n') buf[len-1] = '\0';
        if (mandatory && buf[0] == '\0')
            printf("Поле обязательно.\n");
        else
            break;
    } while (mandatory);
    strcpy(dest, buf);
}

static Contact input_new_contact() {
    Contact c = {0};
    printf("Фамилия: "); read_str(c.name.last_name, sizeof(c.name.last_name), 1);
    printf("Имя: "); read_str(c.name.first_name, sizeof(c.name.first_name), 1);
    printf("Отчество: "); read_str(c.name.middle_name, sizeof(c.name.middle_name), 0);
    printf("Место работы: "); read_str(c.work.place, sizeof(c.work.place), 0);
    printf("Должность: "); read_str(c.work.position, sizeof(c.work.position), 0);
    printf("Телефон: "); read_str(c.phone_num, sizeof(c.phone_num), 0);
    printf("Email: "); read_str(c.email, sizeof(c.email), 0);
    printf("Соцсеть: "); read_str(c.social, sizeof(c.social), 0);
    printf("Мессенджер: "); read_str(c.messenger, sizeof(c.messenger), 0);
    return c;
}

static void print_contact(const Contact *c) {
    printf("--- ID: %d ---\n%s %s", c->id, c->name.last_name, c->name.first_name);
    if (c->name.middle_name[0]) printf(" %s", c->name.middle_name);
    printf("\n");
    if (c->work.place[0]) printf("Работа: %s, %s\n", c->work.place, c->work.position);
    if (c->phone_num[0]) printf("Тел: %s\n", c->phone_num);
    if (c->email[0]) printf("Email: %s\n", c->email);
    if (c->social[0]) printf("Соцсеть: %s\n", c->social);
    if (c->messenger[0]) printf("Мессенджер: %s\n", c->messenger);
    printf("\n");
}

static void load(Contact *list, int *cnt, int *next_id) {
    int fd = open(DATA_FILE, O_RDONLY);
    if (fd == -1) { *cnt = 0; *next_id = 1; return; }
    int saved;
    if (read(fd, &saved, sizeof(int)) != sizeof(int)) saved = 0;
    if (saved > MAX) saved = MAX;
    int i;
    for (i = 0; i < saved && read(fd, &list[i], sizeof(Contact)) == sizeof(Contact); i++);
    *cnt = i;
    close(fd);
    int max_id = 0;
    for (i = 0; i < *cnt; i++) if (list[i].id > max_id) max_id = list[i].id;
    *next_id = max_id + 1;
}

static void save(Contact *list, int cnt) {
    int fd = open(DATA_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) return;
    write(fd, &cnt, sizeof(int));
    for (int i = 0; i < cnt; i++) write(fd, &list[i], sizeof(Contact));
    close(fd);
}
static void print_menu() {
    printf("\n--- Телефонная книга ---\n");
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
    Contact list[MAX];
    int cnt = 0, next_id = 1;
    load(list, &cnt, &next_id);

    int choice;
    do {
        print_menu();
        scanf("%d", &choice);
        while(getchar()!='\n');

        switch (choice) {
            case 1:
                if (cnt == MAX) { printf("Книга полна.\n"); break; }
                add_contact(list, &cnt, &next_id, input_new_contact());
                printf("Добавлен. ID: %d\n", next_id-1);
                break;
            case 2: {
                int id; printf("ID: "); scanf("%d", &id); while(getchar()!='\n');
                const Contact *old = get_contact_by_id(list, cnt, id);
                if (!old) { printf("Нет такого ID.\n"); break; }
                print_contact(old);
                Contact newc = *old;
                printf("Новые данные (Enter - оставить):\n");
                printf("Фамилия [%s]: ", old->name.last_name); read_str(newc.name.last_name, sizeof(newc.name.last_name), 0);
                printf("Имя [%s]: ", old->name.first_name); read_str(newc.name.first_name, sizeof(newc.name.first_name), 0);
                printf("Отчество [%s]: ", old->name.middle_name); read_str(newc.name.middle_name, sizeof(newc.name.middle_name), 0);
                printf("Место работы [%s]: ", old->work.place); read_str(newc.work.place, sizeof(newc.work.place), 0);
                printf("Должность [%s]: ", old->work.position); read_str(newc.work.position, sizeof(newc.work.position), 0);
                printf("Телефон [%s]: ", old->phone_num); read_str(newc.phone_num, sizeof(newc.phone_num), 0);
                printf("Email [%s]: ", old->email); read_str(newc.email, sizeof(newc.email), 0);
                printf("Соцсеть [%s]: ", old->social); read_str(newc.social, sizeof(newc.social), 0);
                printf("Мессенджер [%s]: ", old->messenger); read_str(newc.messenger, sizeof(newc.messenger), 0);
                edit_contact(list, cnt, id, newc);
                printf("Обновлено.\n");
                break;
            }
            case 3: {
                int id; printf("ID: "); scanf("%d", &id); while(getchar()!='\n');
                if (delete_contact(list, &cnt, id)) printf("Удалён.\n");
                else printf("Нет такого ID.\n");
                break;
            }
            case 4:
                if (cnt == 0) printf("Пусто.\n");
                else for (int i = 0; i < cnt; i++) print_contact(&list[i]);
                break;
            case 5: {
                char query[100]; printf("Строка: "); fgets(query, 100, stdin);
                query[strcspn(query, "\n")] = 0;
                int ids[MAX], found = search_by_name(list, cnt, query, ids);
                printf("Найдено: %d\n", found);
                for (int i = 0; i < found; i++) {
                    const Contact *c = get_contact_by_id(list, cnt, ids[i]);
                    if (c) printf("ID %d: %s %s\n", c->id, c->name.last_name, c->name.first_name);
                }
                break;
            }
            case 6: {
                int id; printf("ID: "); scanf("%d", &id); while(getchar()!='\n');
                const Contact *c = get_contact_by_id(list, cnt, id);
                if (c) print_contact(c);
                else printf("Нет такого ID.\n");
                break;
            }
        }
    } while (choice != 0);

    save(list, cnt);
    printf("Данные сохранены.\n");
    return 0;
}