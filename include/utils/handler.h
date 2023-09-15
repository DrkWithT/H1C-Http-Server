#ifndef HANDLER_H
#define HANDLER_H

#include "h1c/reqinfo.h"
#include "h1c/resinfo.h"
#include "utils/handlerctx.h"

/** Typedefs & Structs */

typedef enum handler_status_e
{
    HANDLE_OK,           // for status 200
    HANDLE_BAD_PATH,     // for status 404
    HANDLE_BAD_METHOD,   // for status 501 
    HANDLE_BAD_MIME,     // for status 406
    HANDLE_GENERAL_ERR   // for status 500
} HandlerStatus;

/**
 * @brief An alias for normal handler logic.
 */
typedef HandlerStatus (*HandlerFunc)(HandlerContext *ctx, BaseRequest *req, ResponseObj *res);

/**
 * @brief An alias for error handler logic
 */
typedef HandlerStatus (*FallbackFunc)(HandlerContext *ctx, BaseRequest *req, ResponseObj *res, HandlerStatus prev_status);

/**
 * @brief Contains simple request handler data.
 * @todo Use a union to better store function ptrs. for normal/error handling logic.
 */
typedef struct h1chandler_t
{
    HttpMethod method;     // code of accepted method
    MimeType content_type; // allowed MIME type
    HandlerFunc callback;  // function with normal logic
} H1CHandler;

/** H1CHandler Funcs */

/**
 * @brief Sets up this H1CHandler object with tags, logic, and more.
 * 
 * @param handler
 * @param method
 * @param mime 
 * @param callback Normal handling function ptr. 
 */
void h1chandler_init(H1CHandler *handler, HttpMethod method, MimeType mime, HandlerFunc callback);

/**
 * @brief Verifies if a request can be served by its qualities. The basic algorithm does these checks in order: method then MIME type.
 * 
 * @param handler
 * @param req
 * @returns HandlerStatus OK if valid
 */
HandlerStatus h1chandler_check_req(const H1CHandler *handler, BaseRequest *req);

/**
 * @brief Runs either the normal handler logic based on the request data.
 * 
 * @param handler
 * @param ctx The context object to help serve content.
 * @param req
 * @param res
 * @returns A HandlerStatus value. HANDLE_OK on no error.
 */
HandlerStatus h1chandler_handle(const H1CHandler *handler, const HandlerContext *ctx, const BaseRequest *req, ResponseObj *res);

#endif