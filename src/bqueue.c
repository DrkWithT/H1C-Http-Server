/**
 * @file bqueue.c
 * @author Derek Tan
 * @brief Implements a synchronized queue using an internal singly-linked list.
 * @date 2023-09-22
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "collections/bqueue.h"

/* QueueNode Funcs. */

QueueNode *qnode_create(int data_fd)
{
    QueueNode *node = ALLOC_STRUCT(QueueNode);

    if (node != NULL)
    {
        node->data = data_fd;
        node->next = NULL;
    }

    return node;
}

/* BlockingQueue Funcs. */

bool bqueue_init(BlockedQueue *bqueue, int8_t capacity)
{
    int8_t safe_capacity = (capacity >= BQUEUE_MIN_SIZE && capacity <= BQUEUE_MAX_SIZE)
        ? capacity
        : BQUEUE_MIN_SIZE;
    
    bool lock_ok = pthread_mutex_init(&bqueue->lock, NULL) == 0;
    bqueue->head = NULL;
    bqueue->tip = NULL;
    bqueue->capacity = safe_capacity;
    bqueue->count = 0;

    return lock_ok;
}

void bqueue_destroy(BlockedQueue *bqueue)
{
    if (bqueue_is_empty(bqueue))
        return;

    QueueNode *next_ptr = bqueue->head;

    while (bqueue->head != NULL)
    {
        next_ptr = next_ptr->next;
        free(bqueue->head);
        bqueue->head = next_ptr;
    }
    
    bqueue->tip = NULL;
    bqueue->count = 0;
}

bool bqueue_is_empty(const BlockedQueue *bqueue)
{
    return bqueue->count == 0 || !bqueue->head;
}

bool bqueue_is_full(const BlockedQueue *bqueue)
{
    return bqueue->count >= bqueue->capacity;
}

QueueNode *bqueue_dequeue(BlockedQueue *bqueue)
{
    if (bqueue_is_empty(bqueue))
        return NULL;
    
    QueueNode *node = bqueue->head;

    pthread_mutex_lock(&bqueue->lock);

    bqueue->head = node->next;
    node->next = NULL;

    if (!bqueue->head)
        bqueue->tip = NULL;

    pthread_mutex_unlock(&bqueue->lock);

    return node;
}

bool bqueue_enqueue(BlockedQueue *bqueue, QueueNode *node)
{
    if (!node || bqueue_is_full(bqueue))
        return false;

    pthread_mutex_lock(&bqueue->lock);

    if (!bqueue->head)
    {
        bqueue->head = node;
        bqueue->tip = bqueue->head;
    }
    else
    {
        bqueue->tip->next = node;
        bqueue->tip = bqueue->tip->next;
    }

    bqueue->count++;

    pthread_mutex_unlock(&bqueue->lock);
    return true;
}
