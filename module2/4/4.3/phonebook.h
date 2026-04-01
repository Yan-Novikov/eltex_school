#ifndef PHONEBOOK_H
#define PHONEBOOK_H

/* Структура для хранения ФИО */
typedef struct {
    char last_name[50];
    char first_name[50];
    char middle_name[50];
} FullName;

/* Структура для места работы и должности */
typedef struct {
    char place[100];
    char position[50];
} Work;

typedef struct {
    FullName name;
    Work work;
    char phone_num[20];
    char email[50];
    char social[100];
    char messenger[50];
    int id;
} Contact;

typedef struct Phonebook Phonebook;

Phonebook* phonebook_create(void);

void phonebook_destroy(Phonebook *pb);

int phonebook_get_count(const Phonebook *pb);

int phonebook_add(Phonebook *pb, Contact new_contact);

int phonebook_edit(Phonebook *pb, int id, Contact new_contact);

int phonebook_delete(Phonebook *pb, int id);

int phonebook_search_by_name(const Phonebook *pb, const char *query, int result_ids[]);

const Contact* phonebook_get_by_id(const Phonebook *pb, int id);

void phonebook_print_all(const Phonebook *pb);

void phonebook_print_tree(const Phonebook *pb);

void phonebook_rebalance(Phonebook *pb);

#endif