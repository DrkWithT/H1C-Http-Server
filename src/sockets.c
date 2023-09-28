/**
 * @file sockets.c
 * @author Derek Tan
 * @brief Implements synchronous I/O wrappers for TCP sockets. 
 * @date 2023-09-01
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "basicio/sockets.h"

/** ServerSocket */

void serversocket_init(ServerSocket *svr_sock, const char *host, const char *port, int backlog)
{
    bool ready_flag;
    int temp_fd;
    struct addrinfo config, *option_ptr;
    svr_sock->backlog = backlog;

    // Setup socket configuration of TCP/IPv4 with local address.
    memset(&config, 0, sizeof(config));
    config.ai_family = AF_INET;
    config.ai_socktype = SOCK_STREAM;
    config.ai_flags = AI_PASSIVE;

    ready_flag = getaddrinfo(host, port, &config, &option_ptr) == 0;

    if (!ready_flag)
    {
        svr_sock->fd = -1;
        svr_sock->ready = false;
        svr_sock->closed = true;
        return;
    }

    // Do error checks for socket creation and binding for safer operation...
    temp_fd = socket(option_ptr->ai_family, option_ptr->ai_socktype, option_ptr->ai_protocol);

    if (temp_fd == -1)
    {
        svr_sock->fd = -1;
        svr_sock->ready = false;
        svr_sock->closed = true;
        return;
    }

    svr_sock->fd = temp_fd;
    ready_flag = bind(svr_sock->fd, option_ptr->ai_addr, option_ptr->ai_addrlen) != -1;

    // Dispose intrusive list of socket config options...
    freeaddrinfo(option_ptr);

    if (!ready_flag)
    {
        close(svr_sock->fd);
        svr_sock->fd = -1;
        svr_sock->ready = false;
        svr_sock->closed = true;
    }

    // Set connection timeout of 2.5s to reduce worker stalling.
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 2500;

    setsockopt(svr_sock->fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    svr_sock->ready = ready_flag;
    svr_sock->closed = false;
}

bool serversocket_open(ServerSocket *svr_sock)
{
    if (!svr_sock->ready || svr_sock->closed)
        return false;

    return listen(svr_sock->fd, svr_sock->backlog) != -1;
}

void serversocket_close(ServerSocket *svr_sock)
{
    if (!svr_sock->ready || svr_sock->closed)
        return;
    
    close(svr_sock->fd);
    svr_sock->closed = true;
}

int serversocket_accept(ServerSocket *svr_sock)
{
    if (!svr_sock->ready || svr_sock->closed)
        return -1;
    
    return accept(svr_sock->fd, NULL, NULL);
}

/** ClientSocket */

void clientsocket_init(ClientSocket *cli_sock, int fd)
{
    cli_sock->fd = fd;
    cli_sock->closed = (fd == -1);
}

void clientsocket_close(ClientSocket *cli_sock)
{
    if (cli_sock->closed)
        return;
    
    close(cli_sock->fd);
    cli_sock->closed = true;
}

bool clientsocket_read_line(ClientSocket *cli_sock, char delim, Buffer *dst_buf)
{
    char byte;
    bool read_ok = true;
    bool buffer_ok = true;

    do
    {
        read_ok = recv(cli_sock->fd, &byte, 1, 0) != -1;

        if (!read_ok)
            break;

        if (byte == '\r')
        {
            buffer_ok = true; // NOTE: skip CR since only HTTP LF matters as the delimiter.
        }
        else if (byte != delim)
        {
            buffer_ok = buffer_put(dst_buf, byte);
        }
        else
        {
            buffer_ok = buffer_put(dst_buf, '\0');
            break;
        }
    } while (true);

    return read_ok && buffer_ok;
}

bool clientsocket_read_blob(ClientSocket *cli_sock, int count, Buffer *dst_buf)
{
    int pending_rc = count;
    char byte;
    bool read_ok = true;
    bool buffer_ok = true;

    do
    {
        read_ok = recv(cli_sock->fd, &byte, 1, 0) != -1;

        if (!read_ok)
            break;
        
        buffer_ok = buffer_put(dst_buf, byte);
        pending_rc--;

        if (!buffer_ok)
            break;
    } while (pending_rc > 0);
    
    return read_ok && buffer_ok;
}

bool clientsocket_write_blob(ClientSocket *cli_sock, int count, const Buffer *src_buf)
{
    bool write_ok = true;
    int pending_wc = count;
    int temp_wc = 0;
    const char *data_cursor = src_buf->data;

    do
    {
        temp_wc = send(cli_sock->fd, data_cursor, pending_wc, 0);

        write_ok = temp_wc > 0;
        pending_wc -= temp_wc;
        data_cursor += temp_wc;
    } while (write_ok && pending_wc > 0);

    return pending_wc == 0;
}
