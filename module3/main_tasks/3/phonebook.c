#include "phonebook.h"
#include <string.h>

static int find_contact_index_by_id(const Contact contacts[], int count, int id) {
    for (int i = 0; i < count; i++) {
        if (contacts[i].id == id)
            return i;
    }
    return -1;
}

int add_contact(Contact contacts[], int *count, int *next_id, Contact new_contact) {
    if (*count >= 10)
        return 0;
    new_contact.id = *next_id;
    (*next_id)++;
    contacts[*count] = new_contact;
    (*count)++;
    return 1;
}

int edit_contact(Contact contacts[], int count, int id, Contact new_contact) {
    int index = find_contact_index_by_id(contacts, count, id);
    if (index == -1)
        return 0;
    new_contact.id = contacts[index].id;
    contacts[index] = new_contact;
    return 1;
}

int delete_contact(Contact contacts[], int *count, int id) {
    int index = find_contact_index_by_id(contacts, *count, id);
    if (index == -1)
        return 0;
    for (int i = index; i < *count - 1; i++) {
        contacts[i] = contacts[i + 1];
    }
    (*count)--;
    return 1;
}

int search_by_name(const Contact contacts[], int count, const char *query, int result_ids[]) {
    int found = 0;
    for (int i = 0; i < count; i++) {
        if (strstr(contacts[i].name.last_name, query) != NULL ||
            strstr(contacts[i].name.first_name, query) != NULL) {
            result_ids[found++] = contacts[i].id;
        }
    }
    return found;
}

const Contact* get_contact_by_id(const Contact contacts[], int count, int id) {
    int index = find_contact_index_by_id(contacts, count, id);
    if (index == -1)
        return NULL;
    return &contacts[index];
}