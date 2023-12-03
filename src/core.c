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

void server_core_setup_thrd_states(ServerDriver *server)
{
    // setup producer and workers' state
    lstworker_init(&server->producer_obj, &server->entry_socket, &server->task_queue);
    
    for (int i = 0; i < H1C_WORKER_COUNT; i++)
    {
        srvworker_init(&server->workers[i], i + 1, &server->router, &server->ctx, &server->task_queue, H1C_VERSION_STRING);
    }
}

int server_core_run(ServerDriver *server)
{
    // Initialize all producer & worker state...
    int started_worker_count = 0;
    server_core_setup_thrd_states(server);

    // Try starting producer thread first since the workers require tasks before doing work...
    if (pthread_create(&server->thread_ids[0], NULL, lstworker_run, &server->producer_obj) != 0)
        return started_worker_count;

    // Try starting workers since tasks are possibly available or incoming...
    for (int pthrd_i = 1; pthrd_i < H1C_TOTAL_THREADS; pthrd_i++)
    {
        if (pthread_create(&server->thread_ids[pthrd_i], NULL, run_srvworker, &server->workers[started_worker_count]) != 0)
            break;
        
        started_worker_count++;
    }

    return started_worker_count;
}

void server_core_join(ServerDriver *server, int wthrd_count)
{
    if (!wthrd_count)
        return;

    for (int wthrd_i = 0; wthrd_i < wthrd_count; wthrd_i++)
    {
        if (pthread_join(server->thread_ids[1 + wthrd_i], NULL) != 0)
            break;
    }
}

void server_core_cleanup(ServerDriver *server, int wthrd_count)
{
    // Stop and dispose producer and workers...
    lstworker_end(&server->producer_obj);

    for (int worker_i = 0; worker_i < wthrd_count; worker_i++)
        srvworker_dispose(&server->workers[worker_i]);

    // Wake up workers... they will stop on realization that they must abort.
    fprintf(stdout, "%s log: Signaling workers to quit.\n", H1C_VERSION_STRING);
    pthread_cond_broadcast(&server->task_queue.signaler);

    // Dispose other memory / resources...
    fprintf(stdout, "%s log: Disposing routes and handlers.\n", H1C_VERSION_STRING);
    bqueue_destroy(&server->task_queue);
    rtemap_dispose(&server->router);
    handlerctx_dispose(&server->ctx);
}
