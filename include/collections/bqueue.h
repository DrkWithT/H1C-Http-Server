#ifndef BQUEUE_H
#define BQUEUE_H

#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>

#include "utils/misc.h"

/* Magic Macros */

#define BQUEUE_MIN_SIZE 4
#define BQUEUE_DEFAULT_SIZE 16
#define BQUEUE_MAX_SIZE 32

/* BlockedQueue Structs */

typedef struct qnode_t
{
    int data;  // client-initiated connection fd
    struct qnode_t *next;
} QueueNode;

typedef struct bqueue_t
{
    int8_t count;
    int8_t capacity;
    pthread_mutex_t lock;  // prevents race conditions of queue data
    QueueNode *head;       // first node
    QueueNode *tip;        // last node
} BlockedQueue;

/* QueueNode Funcs. */

QueueNode *qnode_create(int data_fd);

/* BlockingQueue Funcs. */

bool bqueue_init(BlockedQueue *bqueue, int8_t capacity);

void bqueue_destroy(BlockedQueue *bqueue);

bool bqueue_is_empty(const BlockedQueue *bqueue);

bool bqueue_is_full(const BlockedQueue *bqueue);

QueueNode *bqueue_dequeue(BlockedQueue *bqueue);

bool bqueue_enqueue(BlockedQueue *bqueue, QueueNode *node);

#endif
