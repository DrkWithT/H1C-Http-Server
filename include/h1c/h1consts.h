#ifndef H1CONSTS_H
#define H1CONSTS_H

/** Misc. */

#define HTTP_1X_SP ' '
#define HTTP_1X_CR '\r'
#define HTTP_1X_LF '\n'
#define HTTP_1X_COLON ':'
#define HTTP_GMT_FMT "%a, %d %b %Y %H:%M:%S GMT"

/** Schemas */

#define HTTP_1_0 "HTTP/1.0"
#define HTTP_1_1 "HTTP/1.1"

/** Methods */

#define HTTP_METHOD_HEAD "HEAD"
#define HTTP_METHOD_GET "GET"
#define HTTP_METHOD_POST "POST"

/** Checked Headers */

#define HTTP_HEADER_HOST "Host:"
#define HTTP_HEADER_SERVER "Server:"
#define HTTP_HEADER_DATE "Date:"
#define HTTP_HEADER_CONNECTION "Connection:"
#define HTTP_HVALUE_CONN_ALIVE "keep-alive"
#define HTTP_HVALUE_CONN_CLOSE "close"
#define HTTP_HEADER_CTYPE "Content-Type:"
#define HTTP_HEADER_CLEN "Content-Length:"

/** Content_Type MIMEs */

#define MIME_ANY "*/*"
#define MIME_TXT_PLAIN "text/plain"
#define MIME_TXT_HTML "text/html"
#define MIME_TXT_CSS "text/css"
#define MIME_TXT_JS "text/javascript"

/** Statuses */

#define HTTP_STATUS_OK "200"
#define HTTP_STATUS_BAD_REQUEST "400"
#define HTTP_STATUS_UNFOUND "404"
#define HTTP_STATUS_NO_ACCEPT "406"
#define HTTP_STATUS_SERVER_ERR "500"
#define HTTP_STATUS_NO_IMPL "501"

#define HTTP_MSG_OK "OK"
#define HTTP_MSG_BAD_REQUEST "Bad Request"
#define HTTP_MSG_UNFOUND "Not Found"
#define HTTP_MSG_NO_ACCEPT "Not Acceptable"
#define HTTP_MSG_SERVER_ERR "Internal Server Error"
#define HTTP_MSG_NO_IMPL "Not Implemented"

/** Enums */

typedef enum http_method_e
{
    HEAD,
    GET,
    POST,
    ANYTHING,
    UNKNOWN
} HttpMethod;

typedef enum http_schema_e
{
    HTTP_SCHEMA_1_0,
    HTTP_SCHEMA_1_1,
    HTTP_SCHEMA_UNKNOWN
} HttpSchema;

typedef enum mime_type_e
{
    ANY_ANY,
    TXT_PLAIN,
    TXT_HTML,
    TXT_CSS,
    TXT_JS,
    MIME_UNKNOWN
} MimeType;

#endif