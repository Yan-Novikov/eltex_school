#ifndef PHONEBOOK_H
#define PHONEBOOK_H

typedef struct {
    char last_name[50];
    char first_name[50];
    char middle_name[50];
} FullName;

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

int add_contact(Contact contacts[], int *count, int *next_id, Contact new_contact);
int edit_contact(Contact contacts[], int count, int id, Contact new_contact);
int delete_contact(Contact contacts[], int *count, int id);
int search_by_name(const Contact contacts[], int count, const char *query, int result_ids[]);
const Contact* get_contact_by_id(const Contact contacts[], int count, int id);

#endif