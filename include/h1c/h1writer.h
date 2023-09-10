#ifndef H1WRITER_H
#define H1WRITER_H

#include "h1c/h1consts.h"
#include "basicio/buffers.h"
#include "basicio/sockets.h"
#include "h1c/resinfo.h"

/** Macros */

#define DEFAULT_REPLY_BUFSIZE 3096

/** Structs */

/**
 * @brief A state structure for the HTTP/1.x response writer.
 */
typedef struct h1writer_t
{
    ClientSocket *cli_sock_ref;
    Buffer reply_buf;
} ReplyWriter;

/** ReplyWriter Funcs */

void h1writer_init(ReplyWriter *writer, ClientSocket *cli_sock_ref);
void h1writer_dispose(ReplyWriter *writer);
void h1writer_reset(ReplyWriter *writer);

bool h1writer_put_status_line(ReplyWriter *writer, const ResponseObj *resinfo);
bool h1writer_put_header_server(ReplyWriter *writer, const ResponseObj *resinfo);
bool h1writer_put_header_date(ReplyWriter *writer, const ResponseObj *resinfo);
bool h1writer_put_header_keepconn(ReplyWriter *writer, const ResponseObj *resinfo);
bool h1writer_put_header_contype(ReplyWriter *writer, const ResponseObj *resinfo);
bool h1writer_put_header_contlen(ReplyWriter *writer, const ResponseObj *resinfo);
bool h1writer_put_header_blank(ReplyWriter *writer);
bool h1writer_write_body_blob(ReplyWriter *writer, const ResponseObj *resinfo);
bool h1writer_write_out(ReplyWriter *writer);

bool h1writer_put_reply(ReplyWriter *writer, const ResponseObj *resinfo);

#endif