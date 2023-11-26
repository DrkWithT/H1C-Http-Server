#ifndef SRVWORKER_H
#define SRVWORKER_H

#include "collections/bqueue.h"
#include "h1c/h1scanner.h"
#include "h1c/h1writer.h"
#include "utils/routemap.h"

/* Enums */

typedef enum srvworker_state_e
{
    SWORKER_START = 0,
    SWORKER_CONSUME,
    SWORKER_RECV,
    SWORKER_PROCESS,
    SWORKER_SEND,
    SWORKER_RESET,
    SWORKER_END
} ServerWorkerState;

/* ServerWorker */

typedef struct srvworker_t
{
    ServerWorkerState state;  // controlling FSM status for worker actions 

    BaseRequest request;
    ResponseObj response;

    HttpScanner scanner;
    ReplyWriter writer;
    ClientSocket clisock;     // reusable client fd wrapper

    RouteMap *router_ref;     // route to handler tree
    HandlerContext *ctx_ref;  // shared reference to resource table
    BlockedQueue *bqueue_ref; // shared reference to synchronized task queue
} ServerWorker;

/* ServerWorker Funcs. */

void srvworker_init(ServerWorker *srvworker, RouteMap *router_ref, HandlerContext *ctx_ref, BlockedQueue *bqueue_ref, const char *server_name);

/**
 * @brief Special cleanup function for ServerWorker data... Only meant to be used in final server cleanup AFTER the worker thread ends.
 * 
 * @param srvworker 
 */
void srvworker_dispose(ServerWorker *srvworker);

ServerWorkerState srvworker_consume(ServerWorker *srvworker);

ServerWorkerState srvworker_recv(ServerWorker *srvworker);

ServerWorkerState srvworker_process_ok(ServerWorker *srvworker, const BaseRequest *req_ref);

ServerWorkerState srvworker_process_bad(ServerWorker *srvworker, const char *status_str, const char *msg_str, const BaseRequest *req_ref);

ServerWorkerState srvworker_process_all(ServerWorker *srvworker);

ServerWorkerState srvworker_send(ServerWorker *srvworker);

ServerWorkerState srvworker_reset(ServerWorker *srvworker);

void *run_srvworker(void *srvworker_ref);

#endif