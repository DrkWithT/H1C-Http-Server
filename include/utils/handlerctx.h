#ifndef HANDLERCTX_H
#define HANDLERCTX_H

#include "utils/misc.h"
#include "utils/resrctable.h"

/* HandlerContext */

typedef struct handlerctx_t
{
    bool ready;               // if initialization had no errors
    ResourceTable resources;  // static resource hashtable for now
} HandlerContext;

/* HandlerContext Funcs. */

bool handlerctx_init(HandlerContext *handlerctx, uint16_t fcount, const char *fnames[]);
void handlerctx_dispose(HandlerContext *handlerctx);
inline bool handlerctx_ready(const HandlerContext *handlerctx);
const StaticResource *handlerctx_get_resrc(const HandlerContext *handlerctx, const char *fname);

#endif