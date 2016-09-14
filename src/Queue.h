typedef struct {
    int x;
    int y;
} Point;
typedef struct Queue Queue;

Queue *createQueue(int size);
void freeQueue(Queue *q);
int enqueue(Queue *q, Point p);
Point dequeue(Queue *q);
int hasNext(Queue *q);
