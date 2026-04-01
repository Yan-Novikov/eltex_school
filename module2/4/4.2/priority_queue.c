#include "priority_queue.h"
#include <stdlib.h>

PriorityQueue* pq_create(void) {
    PriorityQueue *pq = (PriorityQueue*)malloc(sizeof(PriorityQueue));
    if (pq) pq->head = NULL;
    return pq;
}

void pq_destroy(PriorityQueue *pq) {
    if (!pq) return;
    Node *cur = pq->head;
    while (cur) {
        Node *tmp = cur;
        cur = cur->next;
        free(tmp);
    }
    free(pq);
}

int pq_enqueue(PriorityQueue *pq, int priority, int data) {
    if (!pq || priority < 0 || priority > 255) return -1;

    Node *new_node = (Node*)malloc(sizeof(Node));
    if (!new_node) return -1;
    new_node->data = data;
    new_node->priority = priority;
    new_node->next = NULL;

    if (!pq->head || priority > pq->head->priority) {
        new_node->next = pq->head;
        pq->head = new_node;
        return 0;
    }

    Node *cur = pq->head;
    while (cur->next && cur->next->priority >= priority) {
        cur = cur->next;
    }
    new_node->next = cur->next;
    cur->next = new_node;
    return 0;
}

static int remove_node(PriorityQueue *pq, Node *prev, int *success) {
    Node *target;
    if (prev == NULL) {
        if (!pq->head) {
            if (success) *success = 0;
            return 0;
        }
        target = pq->head;
        pq->head = target->next;
    } else {
        if (!prev->next) {
            if (success) *success = 0;
            return 0;
        }
        target = prev->next;
        prev->next = target->next;
    }
    int data = target->data;
    free(target);
    if (success) *success = 1;
    return data;
}

int pq_dequeue_first(PriorityQueue *pq, int *success) {
    if (!pq || !pq->head) {
        if (success) *success = 0;
        return 0;
    }
    return remove_node(pq, NULL, success);
}

int pq_dequeue_by_priority(PriorityQueue *pq, int priority, int *success) {
    if (!pq || !pq->head) {
        if (success) *success = 0;
        return 0;
    }
    if (pq->head->priority == priority) {
        return remove_node(pq, NULL, success);
    }
    Node *cur = pq->head;
    while (cur->next && cur->next->priority != priority) {
        cur = cur->next;
    }
    if (!cur->next) {
        if (success) *success = 0;
        return 0;
    }
    return remove_node(pq, cur, success);
}

int pq_dequeue_by_max_priority(PriorityQueue *pq, int max_priority, int *success) {
    if (!pq || !pq->head) {
        if (success) *success = 0;
        return 0;
    }
    if (pq->head->priority <= max_priority) {
        return remove_node(pq, NULL, success);
    }
    Node *cur = pq->head;
    while (cur->next && cur->next->priority > max_priority) {
        cur = cur->next;
    }
    if (!cur->next) {
        if (success) *success = 0;
        return 0;
    }
    return remove_node(pq, cur, success);
}

int pq_is_empty(PriorityQueue *pq) {
    return (pq == NULL || pq->head == NULL);
}