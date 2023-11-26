#ifndef LSTWORKER_H
#define LSTWORKER_H

#include "basicio/sockets.h"
#include "collections/bqueue.h"

/* Macros and Enums */

typedef struct listen_worker_t
{
    bool is_listening;          // flag for running
    ServerSocket *srvsock_ref;  // listening socket
    BlockedQueue *bqueue_ref;   // task queue
} ListenWorker;

void lstworker_init(ListenWorker *lstworker, ServerSocket *srvsock_ref, BlockedQueue *bqueue_ref);

void lstworker_end(ListenWorker *lstworker);

void lstworker_work(ListenWorker *lstworker);

void *lstworker_run(void *lstworker_ref);

#endif