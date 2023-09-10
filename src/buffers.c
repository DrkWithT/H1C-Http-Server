/**
 * @file buffers.c
 * @author Derek Tan
 * @brief Implements I/O Buffer functions.
 * @date 2023-08-30
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "basicio/buffers.h"

void buffer_init(Buffer *buf, int capacity)
{
    buf->write_pos = 0;
    buf->read_pos = 0;
    buf->capacity = (capacity > 0) ? capacity : MIN_BUFFER_SIZE;
    buf->data = calloc(buf->capacity, sizeof(int8_t));

    if (!buf->data)
        buf->capacity = 0;
}

void buffer_destroy(Buffer *buf)
{
    if (!buf->data)
        return;
    
    free(buf->data);
    buf->data = NULL;
    buf->capacity = 0;
    buf->read_pos = 0;
    buf->write_pos = 0;
}

bool buffer_grow(Buffer *buf, int new_capacity)
{
    int old_capacity = buf->capacity;
    int clamped_capacity = (new_capacity > old_capacity && new_capacity < INT16_MAX) ? new_capacity : 0;

    if (clamped_capacity == 0 || clamped_capacity <= old_capacity)
        return false;

    char *new_buffer = realloc(buf->data, sizeof(int8_t) * clamped_capacity);

    if (!new_buffer)
        return false;

    memset(new_buffer + old_capacity, '\0', clamped_capacity - old_capacity);
    buf->data = new_buffer;
    buf->capacity = clamped_capacity;

    return true;
}

void buffer_clear(Buffer *buf)
{
    memset(buf->data, '\0', buf->capacity);
    buf->read_pos = 0;
    buf->write_pos = 0;
}

char *buffer_pop(Buffer *buf)
{
    char *popped_data = buf->data;

    buf->data = NULL;
    buf->capacity = 0;
    buf->read_pos = 0;
    buf->write_pos = 0;

    return popped_data;
}

bool buffer_is_full(const Buffer *buf)
{
    return buf->write_pos >= buf->capacity;
}

bool buffer_is_drained(const Buffer *buf)
{
    return buf->read_pos > buf->write_pos;
}

int buffer_get_wpos(const Buffer *buf)
{
    return buf->write_pos;
}

bool buffer_set_wpos(Buffer *buf, int wpos)
{
    bool wpos_valid = wpos >= 0 && wpos < buf->capacity;
    
    if (wpos_valid)
        buf->write_pos = wpos;

    return wpos_valid;
}

int buffer_get_rpos(const Buffer *buf)
{
    return buf->read_pos;
}

bool buffer_set_rpos(Buffer *buf, int rpos)
{
    bool rpos_valid = rpos >= 0 && rpos <= buf->write_pos;

    if (rpos_valid)
        buf->read_pos = rpos;
    
    return rpos_valid;
}

bool buffer_put(Buffer *buf, char byte)
{
    if (buffer_is_full(buf))
        return false;
    
    buf->data[buf->write_pos] = byte;
    buf->write_pos++;

    return true;
}

bool buffer_put_span(Buffer *buf, int count, const char *bytes)
{
    int space_left = buf->capacity - buf->write_pos;
    
    if (count > space_left)
        return false;
    
    int pending_bytes = count;
    const char *cursor = bytes;

    while (pending_bytes > 0 && !buffer_is_full(buf))
    {
        buf->data[buf->write_pos] = *cursor;
        cursor++;
        buf->write_pos++;
        pending_bytes--;
    }

    return pending_bytes == 0;
}

char buffer_get(Buffer *buf)
{
    if (buffer_is_drained(buf))
        return '\0';

    char byte = buf->data[buf->read_pos];
    buf->read_pos++;

    return byte;
}

char *buffer_get_span(Buffer *buf, int count)
{
    if (buffer_is_drained(buf) || count < 0 || count > (buf->write_pos - buf->read_pos))
        return NULL;
    
    int span_size = count + 1;
    char *bytes = calloc(span_size, sizeof(int8_t));

    if (bytes != NULL)
    {
        for (int i = 0; i < span_size; i++)
            bytes[i] = buffer_get(buf);
        
        bytes[span_size - 1] = '\0';
    }

    buf->read_pos += count;

    return bytes;
}

char *buffer_read_delim(Buffer *buf, char delim)
{
    if (buffer_is_drained(buf))
        return NULL;
    
    int count = 0;
    int span_size = 0;
    int space_left = buf->capacity - buf->read_pos;
    const char *data_cursor = buf->data + buf->read_pos;
    char *bytes = NULL;

    // 1. Peek ahead until delimiter or end is found...
    uint8_t byte;

    do
    {
        byte = *data_cursor;
        
        if (byte == delim)
            break;
        
        data_cursor++;
        count++;
        space_left--;
    }
    while (space_left > 0);
    
    // 2. Allocate and fill the delimited buffer.
    span_size = count + 1;
    data_cursor -= count;
    buf->read_pos += count + 1; // Put +1 to skip delimiter.

    bytes = calloc(span_size, sizeof(int8_t));

    if (!bytes)
        return NULL;

    memcpy(bytes, data_cursor, count);
    bytes[span_size - 1] = '\0';

    return bytes;
}
