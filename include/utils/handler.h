#ifndef HANDLER_H
#define HANDLER_H

#include "h1c/reqinfo.h"
#include "h1c/resinfo.h"

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
typedef HandlerStatus (*HandlerFunc)(const BaseRequest *req, ResponseObj *res);

/**
 * @brief An alias for error handler logic
 */
typedef HandlerStatus (*FallbackFunc)(const BaseRequest *req, ResponseObj *res, HandlerStatus prev_status);

/**
 * @brief Contains simple request handler data.
 */
typedef struct h1chandler_t
{
    bool is_fallback;      // if handler is an error handler
    HttpMethod method;     // code of accepted method
    MimeType content_type; // allowed MIME type
    HandlerFunc callback;  // function with normal logic
    FallbackFunc fallback; // function with error handling logic
} H1CHandler;

/** H1CHandler Funcs */

void h1chandler_init(H1CHandler *handler, bool is_fallback, HttpMethod method, MimeType mime, HandlerFunc callback, FallbackFunc fallback);

/**
 * @brief Verifies if a request can be served by its qualities. The basic algorithm does these checks in order: method then MIME type.
 * 
 * @param handler
 * @param req
 * @returns HandlerStatus OK if valid
 */
HandlerStatus h1chandler_check_req(const H1CHandler *handler, BaseRequest *req);

/**
 * @brief Runs either the normal or error handler logic based on the request data. If the request is invalid by the checks in h1chandler_check_req, then the error handling logic runs. Otherwise, the normal logic runs.  
 * 
 * @param handler
 * @param req
 * @param res
 */
void h1chandler_handle(const H1CHandler *handler, const BaseRequest *req, ResponseObj *res);

#endif