/**
 * @file reqinfo.c
 * @author Derek Tan
 * @brief Implements cleanup function for reusable request info structure.
 * @date 2023-09-02
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "h1c/reqinfo.h"

MimeType mime_id_to_code(const char *mime_str)
{
    if (strcmp(mime_str, MIME_ANY) == 0)
        return ANY_ANY;
    else if (strcmp(mime_str, MIME_TXT_PLAIN) == 0)
        return TXT_PLAIN;
    else if (strcmp(mime_str, MIME_TXT_HTML) == 0)
        return TXT_HTML;
    else if (strcmp(mime_str, MIME_TXT_CSS) == 0)
        return TXT_CSS;
    else if (strcmp(mime_str, MIME_TXT_JS) == 0)
        return TXT_JS;

    return MIME_UNKNOWN;
}

void basic_reqinfo_init(BaseRequest *base_req)
{
    base_req->schema_id = HTTP_SCHEMA_1_0;
    base_req->method_id = UNKNOWN;
    base_req->path_str = NULL;
    base_req->host_hstr = NULL;
    base_req->body_blob = NULL;
    base_req->keep_connection = false;
    base_req->mime_type = ANY_ANY;
    base_req->content_len = 0;
}

void basic_reqinfo_clear(BaseRequest *base_req)
{
    base_req->schema_id = HTTP_SCHEMA_1_0;
    base_req->method_id = HEAD;

    if (base_req->path_str != NULL)
    {
        free(base_req->path_str);
        base_req->path_str = NULL;
    }

    if (base_req->host_hstr != NULL)
    {
        free(base_req->host_hstr);
        base_req->host_hstr = NULL;
    }

    base_req->body_blob = NULL;  // NOTE: Unbind blob managed by StaticResource objects instead.

    base_req->keep_connection = false;
    base_req->mime_type = ANY_ANY;
    base_req->content_len = 0;
}
