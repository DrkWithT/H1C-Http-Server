/**
 * @file resinfo.c
 * @author Derek Tan
 * @brief Implements ResponseObj.
 * @date 2023-09-04
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "h1c/resinfo.h"

void resinfo_init(ResponseObj *response, const char *server_name)
{
    response->status_line_len = 0;
    memset(response->status_line, '\0', STATUS_LINE_BUFSIZE);
    response->server_name_ref = server_name;
    response->date = time(NULL);
    response->keep_connection = false;
    response->mime_type = MIME_UNKNOWN;
    response->content_len = 0;
    response->body_blob = NULL;
}

void resinfo_reset(ResponseObj *response, ResponseRstMode mode)
{
    if (mode == RES_RST_ALL)
    {
        response->status_line_len = 0;
        memset(response->status_line, '\0', STATUS_LINE_BUFSIZE);
        response->date = time(NULL);
        response->keep_connection = false;
        response->mime_type = MIME_UNKNOWN;
        response->content_len = 0;
        response->body_blob = NULL;
        return;
    }
    else if (mode == RES_RST_HEADERS)
    {
        response->date = time(NULL);
        response->keep_connection = false;
        response->mime_type = MIME_UNKNOWN;
    }
    else if (mode == RES_RST_PAYLOAD)
    {
        response->content_len = 0;
        response->body_blob = NULL;
    }
}

void resinfo_fill_status_line(ResponseObj *response, const char *schema, const char *code, const char *msg)
{
    response->status_line_len = sprintf(response->status_line, "%s %s %s\r\n", schema, code, msg);
}

void resinfo_set_keep_connection(ResponseObj *response, bool is_persistent)
{
    response->keep_connection = is_persistent;
}

void resinfo_set_mime_type(ResponseObj *response, MimeType mime_type)
{
    response->mime_type = mime_type;
}

void resinfo_set_content_length(ResponseObj *response, int content_length)
{
    response->content_len = content_length;
}

void resinfo_set_body_payload(ResponseObj *response, char *blob)
{
    response->body_blob = blob;
}
