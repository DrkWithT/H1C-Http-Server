/**
 * @file srvworker.c
 * @author Derek Tan
 * @brief Implements the server worker for serving static resources for now.
 * @date 2023-09-26
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "server/srvworker.h"

/* ServerWorker Funcs. */

void srvworker_init(ServerWorker *srvworker, int worker_id, RouteMap *router_ref, HandlerContext *ctx_ref, BlockedQueue *bqueue_ref, const char *server_name)
{
    srvworker->wid = worker_id;
    srvworker->state = SWORKER_START;
    srvworker->must_abort = false;
    basic_reqinfo_init(&srvworker->request);
    resinfo_init(&srvworker->response, server_name);

    /// @note ServerWorker I/O utilities are initialized in the consume function.

    srvworker->router_ref = router_ref;
    srvworker->ctx_ref = ctx_ref;
    srvworker->bqueue_ref = bqueue_ref;
}

void srvworker_dispose(ServerWorker *srvworker)
{
    clientsocket_close(&srvworker->clisock);

    h1scanner_dispose(&srvworker->scanner);
    h1writer_dispose(&srvworker->writer);

    srvworker->router_ref = NULL;
    srvworker->ctx_ref = NULL;
    srvworker->bqueue_ref = NULL;
}

ServerWorkerState srvworker_consume(ServerWorker *srvworker)
{
    pthread_mutex_lock(&srvworker->bqueue_ref->lock);

    while (bqueue_is_empty(srvworker->bqueue_ref))
    {
        pthread_cond_wait(&srvworker->bqueue_ref->signaler, &srvworker->bqueue_ref->lock);
    }

    // The queue has something, so getting a connection task from queue is ok.
    QueueNode *popped_task = bqueue_dequeue(srvworker->bqueue_ref);

    clientsocket_init(&srvworker->clisock, popped_task->data);
    h1scanner_init(&srvworker->scanner, &srvworker->clisock);
    h1writer_init(&srvworker->writer, &srvworker->clisock);

    free(popped_task);

    pthread_mutex_unlock(&srvworker->bqueue_ref->lock);

    return SWORKER_RECV;
}

ServerWorkerState srvworker_recv(ServerWorker *srvworker)
{
    if (!h1scanner_read_reqinfo(&srvworker->scanner, &srvworker->request))
    {
        fprintf(stderr, "Error at %s:%i: \"%s\"", __FILE__, __LINE__, "Read of request failed.");
        return SWORKER_RESET;
    }

    return SWORKER_PROCESS;
}

ServerWorkerState srvworker_process_ok(ServerWorker *srvworker, const BaseRequest *req_ref)
{
    // Get basic request data for handler dispatch.
    const HttpMethod req_method = req_ref->method_id;
    const char *req_url = req_ref->path_str;
    const HttpSchema req_schema = req_ref->schema_id;
    bool conn_persisting = req_ref->keep_connection;

    // Get mutable response referencing ptr. 
    ResponseObj *res_ref = &srvworker->response;
    const RoutedNode *handler_item = rtemap_get(srvworker->router_ref, req_url); /// @note This is a simple fetching (read) operation on the route map, so no synchronization is needed here!

    // Check for handler with resource... 404 if none exist.
    if (!handler_item)
        return srvworker_process_bad(srvworker, HTTP_STATUS_UNFOUND, HTTP_MSG_UNFOUND, req_ref);

    // Match the HTTP schema of the request in the first preparation of the reply. This is to avoid unneeded protocol switching.
    switch (req_schema)
    {
    case HTTP_SCHEMA_1_0:
        resinfo_fill_status_line(res_ref, HTTP_1_0, HTTP_STATUS_OK, HTTP_MSG_OK);
        resinfo_set_keep_connection(res_ref, conn_persisting);
        break;
    case HTTP_SCHEMA_1_1:
        resinfo_fill_status_line(res_ref, HTTP_1_1, HTTP_STATUS_OK, HTTP_MSG_OK);
        resinfo_set_keep_connection(res_ref, conn_persisting);
        break;
    default:
        // I reject unsupported HTTP versions for a simpler implementation... Thus, I should close the connection to minimize errors.
        resinfo_fill_status_line(res_ref, HTTP_1_1, HTTP_STATUS_SERVER_ERR, HTTP_MSG_SERVER_ERR);
        resinfo_set_keep_connection(res_ref, false);
        break;
    }

    // Extract handler object from the fetched routing tree node... I also see its status checks for more specific error handling.
    const H1CHandler *handler_ref = rtdnode_get_handler(handler_item);

    HandlerStatus main_handler_status = (handler_ref->method == req_method)
        ? h1chandler_handle(handler_ref, srvworker->ctx_ref, req_ref, res_ref)
        : HANDLE_BAD_METHOD; // BIG ERROR: unexpected 500 from here because of temp_method != GET...

    // Exit before the error replying code to avoid clobbering the server message. Otherwise, replace the response with an errorneous one.
    if (main_handler_status == HANDLE_OK)
        return SWORKER_SEND;

    resinfo_reset(res_ref, RES_RST_ALL);

    if (main_handler_status == HANDLE_BAD_METHOD)
        return srvworker_process_bad(srvworker, HTTP_STATUS_NO_IMPL, HTTP_MSG_NO_IMPL, req_ref);

    if (main_handler_status == HANDLE_BAD_MIME)
        return srvworker_process_bad(srvworker, HTTP_STATUS_NO_ACCEPT, HTTP_MSG_NO_ACCEPT, req_ref);

    return srvworker_process_bad(srvworker, HTTP_STATUS_SERVER_ERR, HTTP_MSG_SERVER_ERR, req_ref);

    return SWORKER_SEND;
}

ServerWorkerState srvworker_process_bad(ServerWorker *srvworker, const char *status_str, const char *msg_str, const BaseRequest *req_ref)
{
    HttpSchema req_schema = req_ref->schema_id;
    ResponseObj *res_ref = &srvworker->response;

    if (req_schema == HTTP_SCHEMA_1_1)
        resinfo_fill_status_line(res_ref, HTTP_1_1, status_str, msg_str);
    else
        resinfo_fill_status_line(res_ref, HTTP_1_0, status_str, msg_str);;

    resinfo_set_keep_connection(res_ref, req_ref->keep_connection);
    resinfo_set_mime_type(res_ref, TXT_PLAIN);
    resinfo_set_content_length(res_ref, 0);
    resinfo_set_body_payload(res_ref, NULL);

    return SWORKER_SEND;
}

ServerWorkerState srvworker_process_all(ServerWorker *srvworker)
{
    // First check request for initial verification: does it have a Host header?
    const BaseRequest *req_view = &srvworker->request;

    bool has_host = req_view->host_hstr != NULL;

    if (has_host)
        return srvworker_process_ok(srvworker, req_view);

    return srvworker_process_bad(srvworker, HTTP_STATUS_BAD_REQUEST, HTTP_MSG_BAD_REQUEST, req_view);
}

ServerWorkerState srvworker_send(ServerWorker *srvworker)
{
    if (!h1writer_put_reply(&srvworker->writer, &srvworker->response))
    {
        // Show error message for any debugging.
        fprintf(stderr, "Error at %s:%i: \"%s\"", __FILE__, __LINE__, "Write of response failed.");
    }

    return SWORKER_RESET;
}

ServerWorkerState srvworker_reset(ServerWorker *srvworker)
{
    bool conn_persists = srvworker->request.keep_connection;

    // Reset HTTP I/O state to avoid request / response clobbering.
    h1scanner_reset(&srvworker->scanner);
    h1writer_reset(&srvworker->writer);
    basic_reqinfo_clear(&srvworker->request);
    resinfo_reset(&srvworker->response, RES_RST_ALL);

    // After reset, there is a chance that the connection is going to end by "Connection: close". Close this stale connection and free any dynamic memory.
    if (!conn_persists)
    {
        clientsocket_close(&srvworker->clisock);
        h1scanner_dispose(&srvworker->scanner);
        h1writer_dispose(&srvworker->writer);
        return SWORKER_CONSUME;
    }

    return SWORKER_RECV;
}

void *run_srvworker(void *srvworker_ref)
{
    ServerWorker *srvworker = (ServerWorker *) srvworker_ref;

    fprintf(stdout, "Started worker %i\n", srvworker->wid);

    while (srvworker->state != SWORKER_END && !srvworker->must_abort)
    {
        if (srvworker->state == SWORKER_START || srvworker->state == SWORKER_CONSUME)
        {
            srvworker->state = srvworker_consume(srvworker);
        }
        else if (srvworker->state == SWORKER_RECV)
        {
            srvworker->state = srvworker_recv(srvworker);
        }
        else if (srvworker->state == SWORKER_PROCESS)
        {
            srvworker->state = srvworker_process_all(srvworker);
        }
        else if (srvworker->state == SWORKER_SEND)
        {
            srvworker->state = srvworker_send(srvworker);
        }
        else if (srvworker->state == SWORKER_RESET)
        {
            srvworker->state = srvworker_reset(srvworker);
        }
        else
        {
            srvworker->state = SWORKER_END;
        }
    }

    srvworker_dispose(srvworker);

    return NULL;
}
