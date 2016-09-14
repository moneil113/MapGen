#include "Queue.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct Element {
    Point p;
    int active;
} Element;

struct Queue {
    int size;
    Element *arr;
    int front;
    int end;
};

Queue *createQueue(int size) {
    Queue *q = calloc(1, sizeof(Queue));
    q->arr = calloc(size, sizeof(Element));

    q->size = size;
    q->front = 0;
    q->end = 0;

    return q;
}

void freeQueue(Queue *q) {
    free(q->arr);
    free(q);
}

int enqueue(Queue *q, Point p) {
    if (q->arr[(q->end + 1) % q->size].active) {
        return 0;
    }
    else {
        Element e = (Element) {p, 1};
        q->arr[q->end] = e;
        q->end = (q->end + 1) % q->size;
        q->arr[q->end].active = 1;

        return 1;
    }
}

Point dequeue(Queue *q) {
    Element e = q->arr[q->front];
    // Point p = q->arr[q->front].p;
    q->arr[q->front].active = 0;
    q->front = (q->front + 1) % q->size;

    return e.p;
}

int hasNext(Queue *q) {
    return q->end != q->front;
}
