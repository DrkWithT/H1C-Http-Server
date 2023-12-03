#ifndef CORE_H
#define CORE_H

#include "server/lstworker.h"
#include "server/srvworker.h"

/* Magic Macros */

#define H1C_VERSION_STRING "H1C/0.3.0"
#define H1C_DEFAULT_HOSTNAME "127.0.0.1"
#define H1C_DEFAULT_PORT "8000"
#define H1C_DEFAULT_BACKLOG 4
#define H1C_WORKER_COUNT 4
#define H1C_TOTAL_THREADS (H1C_WORKER_COUNT + 1)

typedef struct h1c_core_t
{
    /* Service Utils */

    ServerSocket entry_socket;
    HandlerContext ctx;
    RouteMap router;

    /* Concurrency State */

    pthread_t thread_ids[H1C_TOTAL_THREADS]; // thread pool
    BlockedQueue task_queue; // synchronized queue
    ListenWorker producer_obj; // first pthread state
    ServerWorker workers[H1C_WORKER_COUNT]; // other pthreads' states
} ServerDriver;

bool server_core_init(ServerDriver *server, const char *host_name, const char *port, int backlog);
bool server_core_setup_hdctx(ServerDriver *server, const char *file_names[], uint16_t file_count);
bool server_core_put_handler(ServerDriver *server, const char *path, HttpMethod method, MimeType mime, HandlerFunc callback);
void server_core_setup_thrd_states(ServerDriver *server);
int server_core_run(ServerDriver *server);
void server_core_join(ServerDriver *server, int wthrd_count);
void server_core_cleanup(ServerDriver *server, int wthrd_count);

#endif