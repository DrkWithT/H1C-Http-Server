/**
 * @file h1scanner.c
 * @author Derek Tan
 * @brief Implements the HTTP/1.x request scanner.
 * @date 2023-09-02
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "h1c/h1scanner.h"

void h1scanner_init(HttpScanner *scanner, ClientSocket *cli_sock)
{
    scanner->state = START;
    scanner->cli_sock_ref = cli_sock;
    buffer_init(&scanner->header_buf, MIN_BUFFER_SIZE);
    buffer_init(&scanner->body_buf, MIN_BUFFER_SIZE);
    scanner->buffers_ok = scanner->header_buf.capacity > 0 && scanner->body_buf.capacity > 0;
}

void h1scanner_dispose(HttpScanner *scanner)
{
    scanner->state = STOP;
    scanner->cli_sock_ref = NULL; // NOTE: Unbind the client socket reference so that accidental usage post-close is impossible.
    buffer_destroy(&scanner->header_buf);
    buffer_destroy(&scanner->body_buf);
    scanner->buffers_ok = false;
}

void h1scanner_reset(HttpScanner *scanner)
{
    scanner->state = START;
    buffer_clear(&scanner->header_buf);
    buffer_clear(&scanner->body_buf);
    scanner->buffers_ok = true;
}

bool h1scanner_is_ready(const HttpScanner *scanner)
{
    return scanner->buffers_ok;
}

HttpScannerState h1scanner_method(HttpScanner *scanner, BaseRequest *req_ref)
{
    if (!clientsocket_read_line(scanner->cli_sock_ref, HTTP_1X_SP, &scanner->header_buf))
        return ERROR;
    
    const char *method_str = scanner->header_buf.data;

    if (strncmp(method_str, HTTP_METHOD_HEAD, 4) == 0)
        req_ref->method_id = HEAD;
    else if (strncmp(method_str, HTTP_METHOD_GET, 3) == 0)
        req_ref->method_id = GET;
    else if (strncmp(method_str, HTTP_METHOD_POST, 4) == 0)
        req_ref->method_id = POST;
    else
        req_ref->method_id = UNKNOWN;

    buffer_clear(&scanner->header_buf);

    return EAT_URL;
}

HttpScannerState h1scanner_url(HttpScanner *scanner, BaseRequest *req_ref)
{
    if (!clientsocket_read_line(scanner->cli_sock_ref, HTTP_1X_SP, &scanner->header_buf))
        return ERROR;
    
    char *url_str = buffer_read_delim(&scanner->header_buf, '\0');
    req_ref->path_str = url_str;
    buffer_clear(&scanner->header_buf);
    
    return EAT_SCHEMA;
}

HttpScannerState h1scanner_schema(HttpScanner *scanner, BaseRequest *req_ref)
{
    if (!clientsocket_read_line(scanner->cli_sock_ref, HTTP_1X_LF, &scanner->header_buf))
        return ERROR;
    
    const char *schema_str = scanner->header_buf.data;

    if (strncmp(schema_str, HTTP_1_0, 8) == 0)
        req_ref->schema_id = HTTP_SCHEMA_1_0;
    else if (strncmp(schema_str, HTTP_1_1, 8) == 0)
        req_ref->schema_id = HTTP_SCHEMA_1_1;
    else
        req_ref->schema_id = HTTP_SCHEMA_UNKNOWN;

    buffer_clear(&scanner->header_buf);

    return EAT_HEADER;
}

HttpScannerState h1scanner_header(HttpScanner *scanner, BaseRequest *req_ref)
{
    // 1. Read CRLF delimited header line into buffer first to handle empty line case too!
    if (!clientsocket_read_line(scanner->cli_sock_ref, HTTP_1X_LF, &scanner->header_buf))
        return ERROR;

    // 2. Check for empty line in case of transition to reading body...
    int line_len = strlen(scanner->header_buf.data);

    if (line_len == 0 && req_ref->content_len > 0)
        return EAT_BLOB;
    
    if (line_len == 0 && req_ref->content_len <= 0)
        return STOP;

    // 3. Otherwise, process header line if recognized.
    char *hname_str = buffer_read_delim(&scanner->header_buf, HTTP_1X_SP);
    char *hvalue_str = buffer_read_delim(&scanner->header_buf, '\0');
    // printf("hdr %s = %s\n", hname_str, hvalue_str); // debug!

    if (strcmp(hname_str, HTTP_HEADER_HOST) == 0)
    {
        req_ref->host_hstr = hvalue_str;
    }
    else if (strcmp(hname_str, HTTP_HEADER_CONNECTION) == 0)
    {
        req_ref->keep_connection = strcmp(hvalue_str, HTTP_HVALUE_CONN_ALIVE) == 0;
    }
    else if (strcmp(hname_str, HTTP_HEADER_CTYPE) == 0)
    {
        req_ref->mime_type = mime_id_to_code(hvalue_str);
    }
    else if (strcmp(hname_str, HTTP_HEADER_CLEN) == 0)
    {
        req_ref->content_len = atoi(hvalue_str);
    }
    else
    {
        free(hvalue_str); // NOTE: dispose ignored headers to avoid leaks!
    }

    free(hname_str);
    buffer_clear(&scanner->header_buf);

    return EAT_HEADER;
}

HttpScannerState h1scanner_eat_blob(HttpScanner *scanner, BaseRequest *req_ref)
{
    bool buffer_ok = true;
    int blob_size = req_ref->content_len;

    if (blob_size > scanner->body_buf.capacity)
    {
        buffer_ok = buffer_grow(&scanner->body_buf, blob_size + 1);
        buffer_clear(&scanner->body_buf);
    }

    if (!buffer_ok)
        return ERROR;

    if (!clientsocket_read_blob(scanner->cli_sock_ref, blob_size, &scanner->body_buf))
        return ERROR;

    req_ref->body_blob = buffer_get_span(&scanner->body_buf, blob_size);

    buffer_clear(&scanner->body_buf);

    return STOP;
}

bool h1scanner_read_reqinfo(HttpScanner *scanner, BaseRequest *base_req_ref)
{
    while (scanner->state != STOP && scanner->state != ERROR)
    {
        HttpScannerState state_view = scanner->state; 
        if (state_view == START) scanner->state = EAT_METHOD;
        else if (state_view == EAT_METHOD) scanner->state = h1scanner_method(scanner, base_req_ref);
        else if (state_view == EAT_URL) scanner->state = h1scanner_url(scanner, base_req_ref);
        else if (state_view == EAT_SCHEMA) scanner->state = h1scanner_schema(scanner, base_req_ref);
        else if (state_view == EAT_HEADER) scanner->state = h1scanner_header(scanner, base_req_ref);
        else if (state_view == EAT_BLOB) scanner->state = h1scanner_eat_blob(scanner, base_req_ref);
        else; // Ignore invalid states or STOP to prevent bad flow control.
    }

    return scanner->state != ERROR;
}
