#ifndef BUFFERS_H
#define BUFFERS_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define MIN_BUFFER_SIZE 512

/** Buffer Struct */

typedef struct buffer_t
{
    int write_pos; // pos of next byte to load
    int read_pos;  // pos of next byte to read 
    int capacity;  // buffer size
    char *data;
} Buffer;

/** Buffer Funcs. */

void buffer_init(Buffer *buf, int capacity);
void buffer_destroy(Buffer *buf);
bool buffer_grow(Buffer *buf, int new_capacity);
void buffer_clear(Buffer *buf);
char *buffer_pop(Buffer *buf);
bool buffer_is_full(const Buffer *buf);
bool buffer_is_drained(const Buffer *buf);

int buffer_get_wpos(const Buffer *buf);
bool buffer_set_wpos(Buffer *buf, int wpos);
int buffer_get_rpos(const Buffer *buf);
bool buffer_set_rpos(Buffer *buf, int rpos);

bool buffer_put(Buffer *buf, char byte);
bool buffer_put_span(Buffer *buf, int count, const char *bytes);
char buffer_get(Buffer *buf);
char *buffer_get_span(Buffer *buf, int count);
char *buffer_read_delim(Buffer *buf, char delim);

#endif
