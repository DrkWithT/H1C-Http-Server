/**
 * @file lstworker.c
 * @author Derek Tan
 * @brief Implements a connection listener for the producer thread.
 * @date 2023-09-26
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "server/lstworker.h"

void lstworker_init(ListenWorker *lstworker, ServerSocket *srvsock_ref, BlockedQueue *bqueue_ref)
{
    lstworker->is_listening = true;
    lstworker->srvsock_ref = srvsock_ref;
    lstworker->bqueue_ref = bqueue_ref;
}

void lstworker_end(ListenWorker *lstworker)
{
    lstworker->is_listening = false;
    serversocket_close(lstworker->srvsock_ref);
    /// @note Call bqueue_destroy on end of run!
}

void lstworker_work(ListenWorker *lstworker)
{
    int temp_fd = -1;
    QueueNode *temp_task = NULL;

    if (!serversocket_open(lstworker->srvsock_ref))
        return;

    while (lstworker->is_listening)
    {
        // 1. Accept client connection to possibly handle...
        temp_fd = serversocket_accept(lstworker->srvsock_ref);

        if (temp_fd == -1)
        {
            // Handle listening error... try again in case another connection is pending!
            fprintf(stdout, "worker %i log: Invalid connection fd.\n", 0);
            continue;
        }

        // 2. Check blocking queue for placing any connection as task / reject it...
        temp_task = qnode_create(temp_fd);

        if (!temp_task)
        {
            // Allocation failures may mean a memory overload... Stop ASAP!
            close(temp_fd);
            lstworker_end(lstworker);
            continue;
        }

        if (!bqueue_enqueue(lstworker->bqueue_ref, temp_task))
        {
            fprintf(stdout, "worker %i log: Failed to put task.", 0);
            close(temp_task->data);
            continue;
        }

        pthread_cond_signal(&lstworker->bqueue_ref->signaler);
    }

    /// @note The main server state will handle disposes of the blocking queue, web resources, etc. The listener only shares ownership of the queue, but does NOT own it.
}

void *lstworker_run(void *lstworker_ref)
{
    ListenWorker *lstworker = (ListenWorker *)lstworker_ref;

    fprintf(stdout, "Starting producer.\n");

    lstworker_work(lstworker);

    return NULL;
}
