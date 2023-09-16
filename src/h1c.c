/**
 * @file h1c.c
 * @author Derek Tan
 * @brief Implements the HTTP/1.x server.
 * @note The server will accomodate the client's connection wishes. If the client sends close, the server will close the connection after sending its response. Same logic goes for the keep-alive case.
 * @date 2023-09-05
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "server/h1c.h"

void server_init(H1CServer *server, const char *host_str, const char *port_str, int backlog)
{
    server->state = H1C_IDLE;

    serversocket_init(&server->server_sock, host_str, port_str, backlog);
    // (init client socket later on run!)
    // (init HTTP reader and writer on run!)
    basic_reqinfo_init(&server->req);
    server->req.mime_type = TXT_PLAIN;
    server->req.content_len = 0;
    resinfo_init(&server->res, H1C_APP_NAME);
    // (init handler ctx between server init and run)
    rtemap_init(&server->routes);
}

bool server_setup_ctx(H1CServer *server, const char *fnames[], uint16_t fcount)
{
    return handlerctx_init(&server->ctx, fcount, fnames);
}

bool server_add_handler(H1CServer *server, const char *path, HttpMethod method, MimeType mime, HandlerFunc callback)
{
    RoutedNode *handler_item = rtdnode_create(path, method, mime, callback);

    if (handler_item != NULL)
        return rtemap_put(&server->routes, handler_item);

    return false;
}

void server_run(H1CServer *server)
{
    // 1. Set up connection sockets...
    if (!serversocket_open(&server->server_sock))
        return;

    int client_sock_fd = serversocket_accept(&server->server_sock);

    if (client_sock_fd == -1)
    {
        server_end(server);
        return;
    }

    clientsocket_init(&server->client_sock, client_sock_fd);
    server->state = H1C_START;
    h1scanner_init(&server->msg_reader, &server->client_sock);
    h1writer_init(&server->msg_writer, &server->client_sock);

    // 2. Run FSM logic for req-response-error cycle.
    while (server->state != H1C_STOP && server->state != H1C_ERROR)
    {
        switch (server->state)
        {
        case H1C_START:
            server->state = H1C_RECV;
            break;
        case H1C_RECV:
            server->state = server_st_recv(server);
            break;
        case H1C_PROCESS:
            server->state = server_st_process(server);
            break;
        case H1C_SEND:
            server->state = server_st_send(server);
            break;
        case H1C_DONE:
            server->state = server_st_done(server);
            break;
        case H1C_STOP:
            break;
        case H1C_ERROR:
        default:
            server->state = server_st_error(server);
            break;
        }
    }

    // 3. Cleanup server state: close sockets, etc.
    server_end(server);
}

void server_end(H1CServer *server)
{
    server->state = H1C_STOP;

    clientsocket_close(&server->client_sock);
    serversocket_close(&server->server_sock);

    h1scanner_dispose(&server->msg_reader);
    h1writer_dispose(&server->msg_writer);

    basic_reqinfo_clear(&server->req);
    resinfo_reset(&server->res, RES_RST_ALL);

    rtemap_dispose(&server->routes);
    handlerctx_dispose(&server->ctx);
}

H1CState server_handle_valids(H1CServer *server, const BaseRequest *req)
{
    // 1. Get request data.
    const HttpMethod temp_method = req->method_id;  // Preset HTTP/1.x method ID
    const char *temp_url = req->path_str; // TODO: URL to check later!
    const HttpSchema temp_schema = req->schema_id;  // Code for HTTP/1.0 or HTTP/1.1
    bool temp_persist_flag = req->keep_connection; // Flag of whether to continue serving or not by Connection header

    ResponseObj *reply_ref = &server->res;
    const RoutedNode *handler_item = rtemap_get(&server->routes, temp_url);

    if (!handler_item)
        return server_handle_invalids(server, HTTP_STATUS_UNFOUND, HTTP_MSG_UNFOUND, req);

    switch (temp_schema)
    {
    case HTTP_SCHEMA_1_0:
        resinfo_fill_status_line(reply_ref, HTTP_1_0, HTTP_STATUS_OK, HTTP_MSG_OK);
        resinfo_set_keep_connection(reply_ref, temp_persist_flag);
        break;
    case HTTP_SCHEMA_1_1:
        resinfo_fill_status_line(reply_ref, HTTP_1_1, HTTP_STATUS_OK, HTTP_MSG_OK);
        resinfo_set_keep_connection(reply_ref, temp_persist_flag);
        break;
    default:
        // NOTE: I reject unsupported HTTP versions for a simpler implementation... Thus, I should close the connection to minimize errors.
        resinfo_fill_status_line(reply_ref, HTTP_1_1, HTTP_STATUS_SERVER_ERR, HTTP_MSG_SERVER_ERR);
        resinfo_set_keep_connection(reply_ref, false);
        break;
    }

    const H1CHandler *handler_ref = rtdnode_get_handler(handler_item);

    HandlerStatus main_handler_status = (handler_ref->method == temp_method)
        ? h1chandler_handle(handler_ref, &server->ctx, req, reply_ref)
        : HANDLE_BAD_METHOD; // BIG ERROR: unexpected 500 from here because of temp_method != GET...

    // NOTE: Exits with an OK send from this helper if the response was well made... This is for correctness of response semantics.
    if (main_handler_status == HANDLE_OK)
        return H1C_SEND;

    // NOTE: This extra code exits with an errorneous send.
    resinfo_reset(reply_ref, RES_RST_ALL);

    if (main_handler_status == HANDLE_BAD_PATH)
        return server_handle_invalids(server, HTTP_STATUS_UNFOUND, HTTP_MSG_UNFOUND, req);

    if (main_handler_status == HANDLE_BAD_METHOD)
        return server_handle_invalids(server, HTTP_STATUS_NO_IMPL, HTTP_MSG_NO_IMPL, req);

    if (main_handler_status == HANDLE_BAD_MIME)
        return server_handle_invalids(server, HTTP_STATUS_NO_ACCEPT, HTTP_MSG_NO_ACCEPT, req);

    return server_handle_invalids(server, HTTP_STATUS_SERVER_ERR, HTTP_MSG_SERVER_ERR, req);

    return H1C_SEND;
}

H1CState server_handle_invalids(H1CServer *server, const char *status, const char *status_msg, const BaseRequest *req)
{
    // 1. Get request data.
    HttpSchema temp_schema = server->req.schema_id;
    ResponseObj *reply_ref = &server->res;

    // 2. Prepare response status line.
    if (temp_schema == HTTP_SCHEMA_1_0)
        resinfo_fill_status_line(reply_ref, HTTP_1_0, status, status_msg);
    else
        resinfo_fill_status_line(reply_ref, HTTP_1_1, status, status_msg);

    resinfo_set_keep_connection(reply_ref, false);
    resinfo_set_mime_type(reply_ref, TXT_PLAIN);
    resinfo_set_content_length(reply_ref, 0);
    resinfo_set_body_payload(reply_ref, NULL);

    return H1C_SEND;
}

H1CState server_st_recv(H1CServer *server)
{
    if (!h1scanner_read_reqinfo(&server->msg_reader, &server->req))
        return H1C_ERROR;

    printf("(H1req) = {schema: %i, method: %i, ...}\n", server->req.schema_id, server->req.method_id); // debug

    return H1C_PROCESS;
}

H1CState server_st_process(H1CServer *server)
{
    const BaseRequest *req_view = &server->req;

    // Invalid request checks: find Host header.
    bool has_host = server->req.host_hstr != NULL;  // Weakly checked flag for now, as Host: xx verification is extra complex logic for the toy server.
    if (has_host)
        return server_handle_valids(server, req_view);

    return server_handle_invalids(server, HTTP_STATUS_BAD_REQUEST, HTTP_MSG_BAD_REQUEST, req_view);
}

H1CState server_st_send(H1CServer *server)
{
    if (!h1writer_put_reply(&server->msg_writer, &server->res))
        return H1C_ERROR;
    
    return H1C_DONE;
}

H1CState server_st_done(H1CServer *server)
{
    bool copied_persist_flag = server->req.keep_connection;

    h1scanner_reset(&server->msg_reader);
    h1writer_reset(&server->msg_writer);
    basic_reqinfo_clear(&server->req);
    resinfo_reset(&server->res, RES_RST_ALL);

    if (!copied_persist_flag)
        return H1C_STOP;

    return H1C_START;
}

H1CState server_st_error(H1CServer *server)
{
    printf("Stop on ServerError: (debug info todo)\n");

    return H1C_ERROR;
}
