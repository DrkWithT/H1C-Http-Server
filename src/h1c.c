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
            puts("H1C_RECV");
            server->state = server_st_recv(server);
            break;
        case H1C_PROCESS:
            puts("H1C_PROCESS");
            server->state = server_st_process(server);
            break;
        case H1C_SEND:
            puts("H1C_SEND");
            server->state = server_st_send(server);
            break;
        case H1C_DONE:
            puts("H1C_DONE");
            server->state = server_st_done(server);
            break;
        case H1C_STOP:
            break;
        case H1C_ERROR:
        default:
            puts("H1C_ERROR");
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

    h1scanner_dispose(&server->msg_reader);
    h1writer_dispose(&server->msg_writer);
    clientsocket_close(&server->client_sock);
    serversocket_close(&server->server_sock);
    basic_reqinfo_clear(&server->req);
    resinfo_reset(&server->res);
}

H1CState server_handle_valids(H1CServer *server, const BaseRequest *req)
{
    // 1. Get request data.
    const HttpMethod temp_method = req->method_id;  // Preset HTTP/1.x method ID
    const char *temp_url = req->path_str; // TODO: URL to check later!
    const HttpSchema temp_schema = req->schema_id;  // Code for HTTP/1.0 or HTTP/1.1
    bool temp_persist_flag = req->keep_connection; // Flag of whether to continue serving or not by Connection header

    ResponseObj *reply_ref = &server->res;

    // NOTE: replace this hardcoded route check with a trie, tree, or some D.S mapping routes to resources.
    if (strcmp(temp_url, "/") != 0 && strcmp(temp_url, "/hello") != 0)
    {
        return server_handle_invalids(server, HTTP_STATUS_UNFOUND, HTTP_MSG_UNFOUND, req);
    }

    // 2b. Prepare main response content.
    if (temp_method == HEAD)
    {
        // 2. Prepare response status line.
        if (temp_schema == HTTP_SCHEMA_1_0)
            resinfo_fill_status_line(reply_ref, HTTP_1_0, HTTP_STATUS_OK, HTTP_MSG_OK);
        else
            resinfo_fill_status_line(reply_ref, HTTP_1_1, HTTP_STATUS_OK, HTTP_MSG_OK);

        resinfo_set_keep_connection(reply_ref, temp_persist_flag);
        resinfo_set_mime_type(reply_ref, TXT_PLAIN);
        resinfo_set_content_length(reply_ref, H1C_DEMO_CLEN);
        resinfo_set_body_payload(reply_ref, NULL);
    }
    else if (temp_method == GET)
    {
        // 2. Prepare response status line.
        if (temp_schema == HTTP_SCHEMA_1_0)
            resinfo_fill_status_line(reply_ref, HTTP_1_0, HTTP_STATUS_OK, HTTP_MSG_OK);
        else
            resinfo_fill_status_line(reply_ref, HTTP_1_1, HTTP_STATUS_OK, HTTP_MSG_OK);

        resinfo_set_keep_connection(reply_ref, temp_persist_flag);
        resinfo_set_mime_type(reply_ref, TXT_PLAIN);
        resinfo_set_content_length(reply_ref, H1C_DEMO_CLEN);
        resinfo_set_body_payload(reply_ref, H1C_DEMO_CONTENT);
    }
    else
    {
        // 2c. Handle unknown methods with 501!
        // 2. Prepare response status line.
        if (temp_schema == HTTP_SCHEMA_1_0)
            resinfo_fill_status_line(reply_ref, HTTP_1_0, HTTP_STATUS_NO_IMPL, HTTP_STATUS_NO_IMPL);
        else
            resinfo_fill_status_line(reply_ref, HTTP_1_1, HTTP_STATUS_NO_IMPL, HTTP_STATUS_NO_IMPL);

        resinfo_set_keep_connection(reply_ref, temp_persist_flag);
        resinfo_set_mime_type(reply_ref, TXT_PLAIN);
        resinfo_set_content_length(reply_ref, 0);
    }

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

    return H1C_SEND;
}

H1CState server_st_recv(H1CServer *server)
{
    if (!h1scanner_read_reqinfo(&server->msg_reader, &server->req))
        return H1C_ERROR;

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
    resinfo_reset(&server->res);

    if (!copied_persist_flag)
        return H1C_STOP;

    return H1C_START;
}

H1CState server_st_error(H1CServer *server)
{
    printf("Stop on ServerError: (debug info todo)\n");

    return H1C_ERROR;
}