#include "phonebook.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct TreeNode {
    Contact contact;
    struct TreeNode *left; 
    struct TreeNode *right;
} TreeNode;

struct Phonebook {
    TreeNode *root;           
    int count;                  
    int next_id;               
};

static int compare_names(const FullName *a, const FullName *b) {
    int cmp = strcmp(a->last_name, b->last_name);
    if (cmp != 0) return cmp;
    cmp = strcmp(a->first_name, b->first_name);
    if (cmp != 0) return cmp;
    return strcmp(a->middle_name, b->middle_name);
}

static TreeNode* create_node(const Contact *c) {
    TreeNode *node = (TreeNode*)malloc(sizeof(TreeNode));
    if (!node) return NULL;
    node->contact = *c;
    node->left = node->right = NULL;
    return node;
}

static void free_tree(TreeNode *root) {
    if (!root) return;
    free_tree(root->left);
    free_tree(root->right);
    free(root);
}

static TreeNode* insert_node(TreeNode *root, const Contact *c) {
    if (!root) return create_node(c);
    int cmp = compare_names(&c->name, &root->contact.name);
    if (cmp < 0)
        root->left = insert_node(root->left, c);
    else if (cmp > 0)
        root->right = insert_node(root->right, c);
    else {
        root->contact = *c;
    }
    return root;
}

static TreeNode* find_node_by_id(TreeNode *root, int id) {
    if (!root) return NULL;
    if (root->contact.id == id) return root;
    TreeNode *left = find_node_by_id(root->left, id);
    if (left) return left;
    return find_node_by_id(root->right, id);
}

static int delete_node_by_id(TreeNode **root, int id) {
    if (!*root) return 0;
    if ((*root)->contact.id == id) {
        TreeNode *node = *root;
        if (!node->left && !node->right) {
            free(node);
            *root = NULL;
            return 1;
        }
        if (!node->left) {
            *root = node->right;
            free(node);
            return 1;
        }
        if (!node->right) {
            *root = node->left;
            free(node);
            return 1;
        }
        TreeNode *parent = node;
        TreeNode *succ = node->right;
        while (succ->left) {
            parent = succ;
            succ = succ->left;
        }
        node->contact = succ->contact;
        if (parent == node)
            parent->right = succ->right;
        else
            parent->left = succ->right;
        free(succ);
        return 1;
    }
    if (delete_node_by_id(&(*root)->left, id)) return 1;
    return delete_node_by_id(&(*root)->right, id);
}

static void inorder_collect(TreeNode *root, Contact arr[], int *idx) {
    if (!root) return;
    inorder_collect(root->left, arr, idx);
    arr[(*idx)++] = root->contact;
    inorder_collect(root->right, arr, idx);
}

static TreeNode* build_balanced(Contact arr[], int start, int end) {
    if (start > end) return NULL;
    int mid = (start + end) / 2;
    TreeNode *node = create_node(&arr[mid]);
    if (!node) return NULL;
    node->left = build_balanced(arr, start, mid - 1);
    node->right = build_balanced(arr, mid + 1, end);
    return node;
}

Phonebook* phonebook_create(void) {
    Phonebook *pb = (Phonebook*)malloc(sizeof(Phonebook));
    if (!pb) return NULL;
    pb->root = NULL;
    pb->count = 0;
    pb->next_id = 1;
    return pb;
}

void phonebook_destroy(Phonebook *pb) {
    if (!pb) return;
    free_tree(pb->root);
    free(pb);
}

int phonebook_get_count(const Phonebook *pb) {
    return pb ? pb->count : 0;
}

int phonebook_add(Phonebook *pb, Contact new_contact) {
    if (!pb) return 0;
    new_contact.id = pb->next_id++;
    pb->root = insert_node(pb->root, &new_contact);
    if (!pb->root) return 0;
    pb->count++;
    if (pb->count % 10 == 0)
        phonebook_rebalance(pb);
    return 1;
}

int phonebook_edit(Phonebook *pb, int id, Contact new_contact) {
    if (!pb || !phonebook_get_count(pb)) return 0;
    TreeNode *node = find_node_by_id(pb->root, id);
    if (!node) return 0;
    new_contact.id = id;
    if (compare_names(&new_contact.name, &node->contact.name) == 0) {
        node->contact = new_contact;
        return 1;
    }
    int success = phonebook_delete(pb, id);
    if (!success) return 0;
    success = phonebook_add(pb, new_contact);
    return success;
}

int phonebook_delete(Phonebook *pb, int id) {
    if (!pb || !phonebook_get_count(pb)) return 0;
    TreeNode *node = find_node_by_id(pb->root, id);
    if (!node) return 0;
    int success = delete_node_by_id(&pb->root, id);
    if (success) {
        pb->count--;
        if (pb->count % 10 == 0)
            phonebook_rebalance(pb);
    }
    return success;
}

int phonebook_search_by_name(const Phonebook *pb, const char *query, int result_ids[]) {
    if (!pb || !phonebook_get_count(pb) || !query) return 0;
    TreeNode *stack[100];
    int top = 0;
    int found = 0;
    stack[top++] = pb->root;
    while (top > 0 && found < 100) {
        TreeNode *node = stack[--top];
        if (strstr(node->contact.name.last_name, query) != NULL ||
            strstr(node->contact.name.first_name, query) != NULL) {
            result_ids[found++] = node->contact.id;
        }
        if (node->left) stack[top++] = node->left;
        if (node->right) stack[top++] = node->right;
    }
    return found;
}

const Contact* phonebook_get_by_id(const Phonebook *pb, int id) {
    if (!pb || !phonebook_get_count(pb)) return NULL;
    TreeNode *node = find_node_by_id(pb->root, id);
    return node ? &node->contact : NULL;
}

void phonebook_print_all(const Phonebook *pb) {
    if (!pb || !phonebook_get_count(pb)) {
        printf("Телефонная книга пуста.\n");
        return;
    }
    Contact arr[100];
    int idx = 0;
    inorder_collect(pb->root, arr, &idx);
    for (int i = 0; i < idx; i++) {
        const Contact *c = &arr[i];
        printf("--- ID: %d ---\n", c->id);
        printf("Фамилия: %s\n", c->name.last_name);
        printf("Имя: %s\n", c->name.first_name);
        if (c->name.middle_name[0])
            printf("Отчество: %s\n", c->name.middle_name);
        if (c->work.place[0])
            printf("Место работы: %s\n", c->work.place);
        if (c->work.position[0])
            printf("Должность: %s\n", c->work.position);
        if (c->phone_num[0])
            printf("Телефон: %s\n", c->phone_num);
        if (c->email[0])
            printf("Email: %s\n", c->email);
        if (c->social[0])
            printf("Соцсеть: %s\n", c->social);
        if (c->messenger[0])
            printf("Мессенджер: %s\n", c->messenger);
        printf("\n");
    }
}

static void print_tree_recursive(TreeNode *node, int level, int is_right, const char *prefix) {
    if (!node) return;
    char new_prefix[256];
    if (level == 0) {
        printf("%s──%s (ID:%d)\n", prefix, node->contact.name.last_name, node->contact.id);
        sprintf(new_prefix, "%s    ", prefix);
    } else {
        if (is_right) {
            printf("%s└─R─ %s (ID:%d)\n", prefix, node->contact.name.last_name, node->contact.id);
            sprintf(new_prefix, "%s    ", prefix);
        } else {
            printf("%s└─L─ %s (ID:%d)\n", prefix, node->contact.name.last_name, node->contact.id);
            sprintf(new_prefix, "%s    ", prefix);
        }
    }
    print_tree_recursive(node->right, level + 1, 1, new_prefix);
    print_tree_recursive(node->left,  level + 1, 0, new_prefix);
}

void phonebook_print_tree(const Phonebook *pb) {
    if (!pb || !phonebook_get_count(pb)) {
        printf("Дерево пусто.\n");
        return;
    }
    printf("\n=== Структура дерева ===\n");
    print_tree_recursive(pb->root, 0, 0, "");
}

void phonebook_rebalance(Phonebook *pb) {
    if (!pb || pb->count < 2) return;
    Contact *arr = (Contact*)malloc(pb->count * sizeof(Contact));
    if (!arr) return;
    int idx = 0;
    inorder_collect(pb->root, arr, &idx);
    TreeNode *new_root = build_balanced(arr, 0, pb->count - 1);
    if (new_root) {
        free_tree(pb->root);
        pb->root = new_root;
    }
    free(arr);
}