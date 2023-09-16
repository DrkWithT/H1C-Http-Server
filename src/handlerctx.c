/**
 * @file handlerctx.c
 * @author Derek Tan
 * @brief Implements the request handler context object.
 * @date 2023-09-13
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "utils/handlerctx.h"

/* HandlerContext Funcs. */

bool handlerctx_init(HandlerContext *handlerctx, uint16_t fcount, const char *fnames[])
{
    bool table_ok = restable_init(&handlerctx->resources, fcount);
    bool put_ok = true;

    StaticResource *temp_resrc_ref = NULL;

    for (uint16_t i = 0; i < fcount && put_ok; i++)
    {
        temp_resrc_ref = ALLOC_STRUCT(StaticResource);

        if (!temp_resrc_ref)
        {
            put_ok = false;
            continue;
        }

        if (!statsrc_init(temp_resrc_ref, fnames[i]))
        {
            free(temp_resrc_ref);
            temp_resrc_ref = NULL;
            put_ok = false;
            continue;
        }

        put_ok = restable_put(&handlerctx->resources, fnames[i], temp_resrc_ref);
    }

    handlerctx->ready = table_ok && put_ok;

    return handlerctx->ready;
}

void handlerctx_dispose(HandlerContext *handlerctx)
{
    restable_dispose(&handlerctx->resources);
    handlerctx->ready = false;
}

inline bool handlerctx_ready(const HandlerContext *handlerctx)
{
    return handlerctx->ready;
}

const StaticResource *handlerctx_get_resrc(const HandlerContext *handlerctx, const char *fname)
{
    if (!handlerctx->ready)
        return NULL;

    return restable_get(&handlerctx->resources, fname);
}
