/**
 * @file handler.c
 * @author Derek Tan
 * @brief Implements request handler.
 * @date 2023-09-10
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "utils/handler.h"

void h1chandler_init(H1CHandler *handler, HttpMethod method, MimeType mime, HandlerFunc callback)
{
    handler->method = method;
    handler->content_type = mime;
    handler->callback = callback;
}

HandlerStatus h1chandler_check_req(const H1CHandler *handler, BaseRequest *req)
{
    if (req->method_id != handler->method)
        return HANDLE_BAD_METHOD;

    if (handler->content_type != ANY_ANY && req->mime_type != handler->content_type)
        return HANDLE_BAD_MIME;
    
    return HANDLE_OK; // NOTE: I won't check for invalid request header values for now.
}

HandlerStatus h1chandler_handle(const H1CHandler *handler, const HandlerContext *ctx, const BaseRequest *req, ResponseObj *res)
{
    HandlerStatus check_code = h1chandler_check_req(handler, req);

    if (check_code == HANDLE_OK)
        check_code = handler->callback(ctx, req, res);

    return check_code;
}
