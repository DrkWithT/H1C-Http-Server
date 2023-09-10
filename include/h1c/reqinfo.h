#ifndef REQINFO_H
#define REQINFO_H

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "h1c/h1consts.h"

/** Structs */

typedef struct basic_reqinfo
{
    HttpSchema schema_id; // int code for HTTP/1.x schema name
    HttpMethod method_id; // int code for HTTP/1.x method field
    char *path_str;       // raw URL string... relative for now

    char *host_hstr;      // Host header value
    bool keep_connection; // Connection header flag.
    MimeType mime_type;   // Content-Type header value
    int content_len;      // Content-Length header value
    char *body_blob;      // Main message payload in bytes
} BaseRequest;

/** Helpers & Macros */

MimeType mime_id_to_code(const char *mime_str);

void basic_reqinfo_init(BaseRequest *base_req);

void basic_reqinfo_clear(BaseRequest *base_req);

#endif