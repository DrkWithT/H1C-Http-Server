/**
 * @file core.c
 * @author Derek Tan
 * @brief Implements core logic of my web server.
 * @date 2023-11-25
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "server/core.h"

bool server_core_init(ServerDriver *server, const char *host_name, const char *port, int backlog)
{
    bool bqueue_is_ok = true;

    // setup listening socket
    serversocket_init(&server->entry_socket, host_name, port, backlog);

    // setup synchronized queue
    bqueue_is_ok = bqueue_init(&server->task_queue, H1C_DEFAULT_BACKLOG);

    // setup blank route-handler map
    rtemap_init(&server->router);
    
    /// @note HandlerContext and concurrency utilities may be setup afterward with other helper functions.

    return bqueue_is_ok;
}

bool server_core_setup_hdctx(ServerDriver *server, const char *file_names[], uint16_t file_count)
{
    return handlerctx_init(&server->ctx, file_count, file_names);
}

bool server_core_put_handler(ServerDriver *server, const char *path, HttpMethod method, MimeType mime, HandlerFunc callback)
{
    RoutedNode *handler_node = rtdnode_create(path, method, mime, callback);

    if (!handler_node)
        return false;

    return rtemap_put(&server->router, handler_node);
}

bool server_core_setup_thrd_states(ServerDriver *server)
{
    // setup producer and workers' state
    lstworker_init(&server->producer_obj, &server->entry_socket, &server->task_queue);
    
    for (int i = 0; i < H1C_WORKER_COUNT; i++)
    {
        srvworker_init(&server->workers[i], i + 1, &server->router, &server->ctx, &server->task_queue, H1C_VERSION_STRING);
    }

    return true;
}

void server_core_run(ServerDriver *server)
{
    // Try starting producer thread first since the workers require tasks before doing work...
    if (pthread_create(&server->thread_ids[0], NULL, lstworker_run, &server->producer_obj) != 0)
        return;

    if (pthread_join(server->thread_ids[0], NULL) != 0)
        return;

    // Try starting workers since tasks are possibly available or incoming...
    for (int pthrd_i = 1; pthrd_i < H1C_TOTAL_THREADS; pthrd_i++)
    {
        if (pthread_create(&server->thread_ids[pthrd_i], NULL, run_srvworker, &server->workers[pthrd_i - 1]) != 0)
            break;

        if (pthread_join(server->thread_ids[pthrd_i], NULL) != 0)
            break;
    }
}

void server_core_cleanup(ServerDriver *server)
{
    // Stop and dispose producer and workers...
    lstworker_end(&server->producer_obj);

    for (int worker_i = 1; worker_i < H1C_WORKER_COUNT; worker_i++)
    {
        server->workers[worker_i].must_abort = true;
    }

    // Each server worker times out its socket in 2.5s, so waiting a little longer for them to realize they should abort is ok.
    fprintf(stdout, "%s:%d Log: waiting for workers to halt.", __FILE__, __LINE__);
    sleep(H1C_WORKER_COUNT * 3); // Worst case: each of the 4 workers has pending tasks each for 2.5s in order.

    // Dispose other memory / resources...
    rtemap_dispose(&server->router);
    handlerctx_dispose(&server->ctx);
}
