/**
 * @file h1writer.c
 * @author Derek Tan
 * @brief Implements ReplyWriter.
 * @date 2023-09-04
 *
 * @copyright Copyright (c) 2023
 * 
 */

#include "h1c/h1writer.h"

void h1writer_init(ReplyWriter *writer, ClientSocket *cli_sock_ref)
{
    writer->cli_sock_ref = cli_sock_ref;
    buffer_init(&writer->reply_buf, DEFAULT_REPLY_BUFSIZE);
}

void h1writer_dispose(ReplyWriter *writer)
{
    writer->cli_sock_ref = NULL;
    buffer_destroy(&writer->reply_buf);
}

void h1writer_reset(ReplyWriter *writer)
{
    buffer_clear(&writer->reply_buf);
}

bool h1writer_put_status_line(ReplyWriter *writer, const ResponseObj *resinfo)
{
    int status_line_len = resinfo->status_line_len;

    return buffer_put_span(&writer->reply_buf, status_line_len, resinfo->status_line);
}

bool h1writer_put_header_server(ReplyWriter *writer, const ResponseObj *resinfo)
{
    bool put_ok = true;
    int offset_step = 0;
    int cursor_offset = buffer_get_wpos(&writer->reply_buf);
    char *write_cursor = writer->reply_buf.data + cursor_offset;

    offset_step = sprintf(write_cursor, "%s %s\r\n", HTTP_HEADER_SERVER, resinfo->server_name_ref);

    put_ok = offset_step > 0;

    if (put_ok)
        buffer_set_wpos(&writer->reply_buf, cursor_offset + offset_step);

    return put_ok;
}

bool h1writer_put_header_date(ReplyWriter *writer, const ResponseObj *resinfo)
{
    Buffer *buf_ref = &writer->reply_buf;
    int total_offset = buffer_get_wpos(buf_ref);
    int offset_step = total_offset;
    int buf_margin = buf_ref->capacity - offset_step;
    char *write_cursor = buf_ref->data + offset_step;

    // Begin header with Date: ...
    offset_step = sprintf(write_cursor, "%s ", HTTP_HEADER_DATE);

    if (offset_step < 0)
        return false;

    buf_margin -= offset_step;
    write_cursor += offset_step;
    total_offset += offset_step;

    struct tm* gmt_date_ref = gmtime(&resinfo->date);

    if (!gmt_date_ref)
        return false;

    // Put GMT formatted time as text into Date header line...
    offset_step = strftime(write_cursor, buf_margin, HTTP_GMT_FMT, gmt_date_ref);

    if (offset_step < 0)
        return false;

    buf_margin -= offset_step;
    write_cursor += offset_step;
    total_offset += offset_step;

    // End header line with CRLF...
    offset_step = sprintf(write_cursor, "\r\n");

    if (offset_step < 0)
        return false;
    
    buf_margin -= offset_step;
    buf_margin += offset_step;
    total_offset += offset_step;

    return buffer_set_wpos(buf_ref, total_offset);
}

bool h1writer_put_header_keepconn(ReplyWriter *writer, const ResponseObj *resinfo)
{
    bool put_ok = true;
    int offset_step = 0;
    int cursor_offset = buffer_get_wpos(&writer->reply_buf);
    char *write_cursor = writer->reply_buf.data + cursor_offset;

    if (resinfo->keep_connection)
        offset_step = sprintf(write_cursor, "%s %s\r\n", HTTP_HEADER_CONNECTION, HTTP_HVALUE_CONN_ALIVE);
    else
        offset_step = sprintf(write_cursor, "%s %s\r\n", HTTP_HEADER_CONNECTION, HTTP_HVALUE_CONN_CLOSE);

    put_ok = offset_step > 0;

    if (put_ok)
        buffer_set_wpos(&writer->reply_buf, cursor_offset + offset_step);

    return put_ok;
}

bool h1writer_put_header_contype(ReplyWriter *writer, const ResponseObj *resinfo)
{
    bool put_ok = true;
    int offset_step = 0;
    int cursor_offset = buffer_get_wpos(&writer->reply_buf);
    char *write_cursor = writer->reply_buf.data + cursor_offset;

    if (resinfo->mime_type == TXT_PLAIN)
        offset_step = sprintf(write_cursor, "%s %s\r\n", HTTP_HEADER_CTYPE, MIME_TXT_PLAIN);
    else if (resinfo->mime_type == TXT_HTML)
        offset_step = sprintf(write_cursor, "%s %s\r\n", HTTP_HEADER_CTYPE, MIME_TXT_HTML);
    else if (resinfo->mime_type == TXT_CSS)
        offset_step = sprintf(write_cursor, "%s %s\r\n", HTTP_HEADER_CTYPE, MIME_TXT_CSS);
    else if (resinfo->mime_type == TXT_JS)
        offset_step = sprintf(write_cursor, "%s %s\r\n", HTTP_HEADER_CTYPE, MIME_TXT_JS);
    else
        offset_step = sprintf(write_cursor, "%s %s\r\n", HTTP_HEADER_CTYPE, MIME_TXT_PLAIN);

    put_ok = offset_step > 0;

    if (put_ok)
        buffer_set_wpos(&writer->reply_buf, cursor_offset + offset_step);

    return put_ok;
}

bool h1writer_put_header_contlen(ReplyWriter *writer, const ResponseObj *resinfo)
{
    bool put_ok = true;
    int offset_step = 0;
    int cursor_offset = buffer_get_wpos(&writer->reply_buf);
    char *write_cursor = writer->reply_buf.data + cursor_offset;

    offset_step = sprintf(write_cursor, "%s %i\r\n", HTTP_HEADER_CLEN, resinfo->content_len);

    put_ok = offset_step > 0;

    if (put_ok)
        buffer_set_wpos(&writer->reply_buf, cursor_offset + offset_step);

    return put_ok;
}

bool h1writer_put_header_blank(ReplyWriter *writer)
{
    bool put_ok = true;
    int offset_step = 0;
    int cursor_offset = buffer_get_wpos(&writer->reply_buf);
    char *write_cursor = writer->reply_buf.data + cursor_offset;

    offset_step = sprintf(write_cursor, "\r\n");

    put_ok = offset_step > 0;

    if (put_ok)
        buffer_set_wpos(&writer->reply_buf, cursor_offset + offset_step);

    return put_ok;
}

bool h1writer_write_body_blob(ReplyWriter *writer, const ResponseObj *resinfo)
{
    int blob_size = resinfo->content_len;
    const Buffer *res_buf_view = &writer->reply_buf;

    return clientsocket_write_blob(writer->cli_sock_ref, blob_size, res_buf_view);
}

bool h1writer_write_out(ReplyWriter *writer)
{
    const Buffer *res_buf_view = &writer->reply_buf;
    int blob_size = writer->reply_buf.write_pos;

    return clientsocket_write_blob(writer->cli_sock_ref, blob_size, res_buf_view);
}

bool h1writer_put_reply(ReplyWriter *writer, const ResponseObj *resinfo)
{
    bool body_load_ok = true;
    bool write_ok = true;

    if (!h1writer_put_status_line(writer, resinfo))
        return false;

    if (!h1writer_put_header_server(writer, resinfo))
        return false;

    if (!h1writer_put_header_date(writer, resinfo))
        return false;

    if (!h1writer_put_header_keepconn(writer, resinfo))
        return false;
    
    if (!h1writer_put_header_contype(writer, resinfo))
        return false;

    if (!h1writer_put_header_contlen(writer, resinfo))
        return false;

    if (!h1writer_put_header_blank(writer))
        return false;

    if (resinfo->body_blob != NULL)
        body_load_ok = buffer_put_span(&writer->reply_buf, resinfo->content_len, resinfo->body_blob);

    write_ok = h1writer_write_out(writer);

    buffer_clear(&writer->reply_buf); // clear buffer of sent status line

    return write_ok && body_load_ok;
}
