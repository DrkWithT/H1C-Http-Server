#ifndef H1C_H
#define H1C_H

#include "h1c/h1scanner.h"
#include "h1c/h1writer.h"
#include "utils/routemap.h"

/** Magic Macros */

#define H1C_APP_NAME "H1C/0.2.0"
#define H1C_HOSTNAME "localhost"
#define H1C_DEFAULT_PORT "8080"

/** Enums */

/**
 * @brief FSM state for H1CServer.
 * @todo If time permits, this should be moved to a "worker" state struct for later thread-based concurrency.
 */
typedef enum h1c_state_e
{
    H1C_IDLE,
    H1C_START,
    H1C_RECV,
    H1C_PROCESS,
    H1C_SEND,
    H1C_DONE,
    H1C_STOP,
    H1C_ERROR
} H1CState;

/** Struct */

/**
 * @brief Stores server running state and assumes only one test connection served by the main control thread... for now.
 */
typedef struct h1c_server_t
{
    H1CState state;            // finite state value

    ServerSocket server_sock;  // listening endpoint
    ClientSocket client_sock;  // client-to-server endpoint

    HttpScanner msg_reader;    // reads and parses HTTP/1.x requests
    ReplyWriter msg_writer;    // writes buffered HTTP/1.x responses

    BaseRequest req;           // reusable HTTP request storage 
    ResponseObj res;           // reusable HTTP response storage

    HandlerContext ctx;        // storage for extra handler resources
    RouteMap routes;           // request routing BST
} H1CServer;

void server_init(H1CServer *server, const char *host_str, const char *port_str, int backlog);
bool server_setup_ctx(H1CServer *server, const char *fnames[], uint16_t fcount);
bool server_add_handler(H1CServer *server, const char *path, HttpMethod method, MimeType mime, HandlerFunc callback);
void server_run(H1CServer *server);
void server_end(H1CServer *server);

H1CState server_handle_valids(H1CServer *server, const BaseRequest *req);
H1CState server_handle_invalids(H1CServer *server, const char *status, const char *status_msg, const BaseRequest *req);

H1CState server_st_recv(H1CServer *server);
H1CState server_st_process(H1CServer *server);
H1CState server_st_send(H1CServer *server);
H1CState server_st_done(H1CServer *server);
H1CState server_st_error(H1CServer *server);

#endif