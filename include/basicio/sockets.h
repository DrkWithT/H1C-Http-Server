#ifndef SOCKETS_H
#define SOCKETS_H

#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "basicio/buffers.h"

/** ServerSocket */

typedef struct svr_socket_t
{
    int fd;
    int backlog;
    bool ready;
    bool closed;
} ServerSocket;

void serversocket_init(ServerSocket *svr_sock, const char *host, const char *port, int backlog);
bool serversocket_open(ServerSocket *svr_sock);
void serversocket_close(ServerSocket *svr_sock);
int serversocket_accept(ServerSocket *svr_sock);

/** ClientSocket */

typedef struct cli_socket_t
{
    int fd;
    bool closed;
} ClientSocket;

void clientsocket_init(ClientSocket *cli_sock, int fd);
void clientsocket_close(ClientSocket *cli_sock);
bool clientsocket_read_line(ClientSocket *cli_sock, char delim, Buffer *dst_buf);
bool clientsocket_read_blob(ClientSocket *cli_sock, int count, Buffer *dst_buf);
bool clientsocket_write_blob(ClientSocket *cli_sock, int count, const Buffer *src_buf);

#endif