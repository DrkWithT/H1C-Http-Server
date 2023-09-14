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

void h1chandler_init(H1CHandler *handler, bool is_fallback, HttpMethod method, MimeType mime, HandlerFunc callback, FallbackFunc fallback)
{
    handler->is_fallback = is_fallback;
    handler->method = method;
    handler->content_type = mime;
    handler->callback = callback;
    handler->fallback = fallback;
}

HandlerStatus h1chandler_check_req(const H1CHandler *handler, BaseRequest *req)
{
    // NOTE: Error handlers accept any request for flexibility!
    if (handler->is_fallback)
        return HANDLE_OK;

    if (req->method_id != handler->method)
        return HANDLE_BAD_METHOD;
    
    if (req->mime_type != handler->content_type)
        return HANDLE_BAD_MIME;
    
    return HANDLE_OK; // NOTE: I won't check for invalid request header values for now.
}

void h1chandler_handle(const HandlerContext *ctx, const H1CHandler *handler, const BaseRequest *req, ResponseObj *res)
{
    HandlerStatus check_code = h1chandler_check_req(handler, req);

    if (handler->is_fallback)
    {
        return handler->fallback(ctx, req, res, check_code);
    }

    if (check_code == HANDLE_OK)
    {
        check_code = handler->callback(ctx, req, res);
    }
    
    // Normal code can also fail too, so let's handle that for correctness.
    if (check_code != HANDLE_OK)
    {
        check_code = handler->fallback(ctx, req, res, check_code);
    }

    return check_code;
}
