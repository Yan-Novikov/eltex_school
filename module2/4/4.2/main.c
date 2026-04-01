#include <stdio.h>
#include <stdlib.h>
#include "priority_queue.h"

void print_all(PriorityQueue *pq) {
    if (!pq || !pq->head) {
        printf("Очередь пуста\n");
        return;
    }
    printf("Все элементы очереди (приоритет, значение):\n");
    Node *cur = pq->head;
    while (cur) {
        printf("(%d, %d) ", cur->priority, cur->data);
        cur = cur->next;
    }
    printf("\n");
}

int main() {
    PriorityQueue *pq = pq_create();
    if (!pq) {
        printf("Ошибка создания очереди\n");
        return 1;
    }

    int choice;

    do {
        printf("\n--- Меню ---\n");
        printf("1. Добавить число\n");
        printf("2. Извлечь первое (наивысший приоритет)\n");
        printf("3. Извлечь с указанным приоритетом\n");
        printf("4. Извлечь с приоритетом не выше заданного\n");
        printf("5. Проверить, пуста ли очередь\n");
        printf("6. Извлечь и вывести все элементы с указанным приоритетом\n");
        printf("7. Вывод всей очереди\n");
        printf("0. Выход\n");
        printf("Выбор: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: {
                int priority, data;
                printf("Введите число: ");
                scanf("%d", &data);
                printf("Введите приоритет (0-255): ");
                scanf("%d", &priority);
                if (pq_enqueue(pq, priority, data) == 0)
                    printf("Число %d добавлено с приоритетом %d\n", data, priority);
                else
                    printf("Ошибка: неверный приоритет (допустимо 0-255)\n");
                break;
            }
            case 2: {
                int success;
                int data = pq_dequeue_first(pq, &success);
                if (success)
                    printf("Извлечено число: %d\n", data);
                else
                    printf("Очередь пуста\n");
                break;
            }
            case 3: {
                int priority;
                printf("Введите приоритет для извлечения: ");
                scanf("%d", &priority);
                int success;
                int data = pq_dequeue_by_priority(pq, priority, &success);
                if (success)
                    printf("Извлечено число: %d\n", data);
                else
                    printf("Нет чисел с приоритетом %d\n", priority);
                break;
            }
            case 4: {
                int max_priority;
                printf("Введите максимальный приоритет: ");
                scanf("%d", &max_priority);
                int success;
                int data = pq_dequeue_by_max_priority(pq, max_priority, &success);
                if (success)
                    printf("Извлечено число: %d\n", data);
                else
                    printf("Нет чисел с приоритетом <= %d\n", max_priority);
                break;
            }
            case 5:
                if (pq_is_empty(pq))
                    printf("Очередь пуста\n");
                else
                    printf("Очередь не пуста\n");
                break;
            case 6: {
                int priority;
                printf("Введите приоритет: ");
                scanf("%d", &priority);
                printf("Элементы с приоритетом %d: ", priority);
                int count = 0;
                int success;
                int data;
                while ((data = pq_dequeue_by_priority(pq, priority, &success)) && success) {
                    printf("%d ", data);
                    count++;
                }
                if (count == 0)
                    printf("нет");
                printf("\n");
                break;
            }
            case 7:
                print_all(pq);
                break;
            case 0:
                printf("Выход\n");
                break;
            default:
                printf("Неверный выбор\n");
        }
    } while (choice != 0);

    pq_destroy(pq);
    return 0;
}