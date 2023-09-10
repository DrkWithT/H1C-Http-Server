#ifndef RESINFO_H
#define RESINFO_H

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "h1c/h1consts.h"

/** Macros */

#define STATUS_LINE_BUFSIZE 64

/** Struct */

/**
 * @brief A reusable and mutable response data structure for this server.
 */
typedef struct resinfo_t
{
    int status_line_len;
    char status_line[STATUS_LINE_BUFSIZE];  // Stores HTTP/1.x status line as "schema SP status SP msg"
    const char *server_name_ref;  // Unbinds later, but stores a ptr. to server name
    time_t date;            // Date: <GMT> header value
    bool keep_connection;   // Connection header flag
    MimeType mime_type;     // Content-Type header value
    int content_len;        // Content-Length header value
    char *body_blob;        // Main message payload in bytes
} ResponseObj;

void resinfo_init(ResponseObj *response, const char *server_name);
void resinfo_reset(ResponseObj *response);

void resinfo_fill_status_line(ResponseObj *response, const char *schema, const char *code, const char *msg);
void resinfo_set_keep_connection(ResponseObj *response, bool is_persistent);
void resinfo_set_mime_type(ResponseObj *response, MimeType mime_type);
void resinfo_set_content_length(ResponseObj *response, int content_length);
void resinfo_set_body_payload(ResponseObj *response, char *blob);

#endif