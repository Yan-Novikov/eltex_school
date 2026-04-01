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
    int id;
    FullName name;
    Work work;
    char phone[20];
    char email[50];
    char social[100];
    char messenger[50];
} Contact;

typedef struct Node {
    Contact data;
    struct Node *prev;
    struct Node *next;
} Node;

void init_list(void);

int add_contact(Contact new_contact);

int edit_contact(int id, Contact new_data);

int delete_contact(int id);

int search_by_name(const char *query, int result_ids[]);

const Contact* get_contact_by_id(int id);

Node* get_head(void);
Node* get_next(Node *current);

void free_list(void);

#endif