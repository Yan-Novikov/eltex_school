#include "phonebook.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static Node *head = NULL;
static int next_id = 1;

static int compare_contacts(const Contact *a, const Contact *b) {
    int cmp = strcmp(a->name.last_name, b->name.last_name);
    if (cmp != 0) return cmp;
    cmp = strcmp(a->name.first_name, b->name.first_name);
    if (cmp != 0) return cmp;
    return strcmp(a->name.middle_name, b->name.middle_name);
}

static Node* find_node_by_id(int id) {
    Node *cur = head;
    while (cur != NULL) {
        if (cur->data.id == id)
            return cur;
        cur = cur->next;
    }
    return NULL;
}

static void insert_sorted(Node *new_node) {
    if (head == NULL) {
        head = new_node;
        new_node->prev = NULL;
        new_node->next = NULL;
        return;
    }

    if (compare_contacts(&new_node->data, &head->data) < 0) {
        new_node->next = head;
        new_node->prev = NULL;
        head->prev = new_node;
        head = new_node;
        return;
    }

    Node *cur = head;
    while (cur->next != NULL && compare_contacts(&new_node->data, &cur->next->data) >= 0) {
        cur = cur->next;
    }

    new_node->next = cur->next;
    new_node->prev = cur;
    if (cur->next != NULL)
        cur->next->prev = new_node;
    cur->next = new_node;
}

static Node* remove_node(Node *node) {
    if (node == NULL) return NULL;

    if (node->prev != NULL)
        node->prev->next = node->next;
    else
        head = node->next;

    if (node->next != NULL)
        node->next->prev = node->prev;

    node->prev = NULL;
    node->next = NULL;
    return node;
}

static int is_id_unique(int id) {
    return find_node_by_id(id) == NULL;
}

void init_list(void) {
    head = NULL;
    next_id = 1;
}

int add_contact(Contact new_contact) {
    Node *new_node = (Node*)malloc(sizeof(Node));
    if (new_node == NULL) return -1;

    new_node->data = new_contact;

    int candidate_id = next_id;
    while (!is_id_unique(candidate_id)) {
        candidate_id++;
    }
    new_node->data.id = candidate_id;
    next_id = candidate_id + 1;

    new_node->prev = NULL;
    new_node->next = NULL;
    insert_sorted(new_node);
    return new_node->data.id;
}

int edit_contact(int id, Contact new_data) {
    Node *node = find_node_by_id(id);
    if (node == NULL) return 0;

    int old_id = node->data.id;
    remove_node(node);
    node->data = new_data;
    node->data.id = old_id;
    insert_sorted(node);
    return 1;
}

int delete_contact(int id) {
    Node *node = find_node_by_id(id);
    if (node == NULL) return 0;

    remove_node(node);
    free(node);
    return 1;
}

int search_by_name(const char *query, int result_ids[]) {
    int found = 0;
    Node *cur = head;
    while (cur != NULL) {
        if (strstr(cur->data.name.last_name, query) != NULL ||
            strstr(cur->data.name.first_name, query) != NULL) {
            result_ids[found++] = cur->data.id;
        }
        cur = cur->next;
    }
    return found;
}

const Contact* get_contact_by_id(int id) {
    Node *node = find_node_by_id(id);
    return (node != NULL) ? &node->data : NULL;
}

Node* get_head(void) {
    return head;
}

Node* get_next(Node *current) {
    return (current != NULL) ? current->next : NULL;
}

void free_list(void) {
    Node *cur = head;
    while (cur != NULL) {
        Node *next = cur->next;
        free(cur);
        cur = next;
    }
    head = NULL;
    next_id = 1;
}