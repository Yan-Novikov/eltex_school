#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

typedef struct Node {
    int data;
    int priority;
    struct Node *next;
} Node;

typedef struct PriorityQueue {
    Node *head;
} PriorityQueue;

PriorityQueue* pq_create(void);
void pq_destroy(PriorityQueue *pq);

int pq_enqueue(PriorityQueue *pq, int priority, int data);

int pq_dequeue_first(PriorityQueue *pq, int *success);
int pq_dequeue_by_priority(PriorityQueue *pq, int priority, int *success);
int pq_dequeue_by_max_priority(PriorityQueue *pq, int min_priority, int *success);

int pq_is_empty(PriorityQueue *pq);

#endif