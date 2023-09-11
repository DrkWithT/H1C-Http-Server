#ifndef MYURL_H
#define MYURL_H

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "utils/myhash.h"

/* Macros */

#define URLPATH_MIN_CAPACITY 4

/* Structs */

/**
 * @brief Reusable storage for URL path data. It stores hash numbers per path term. Query strings are ignored.
 */
typedef struct urlpath_t
{
    int next_pos;     // next empty index or term count
    int capacity;     // total spaces
    size_t *entries;  // stores URL term hashes
} URLPath;

/* URLObj Funcs. */

bool urlpath_init(URLPath *path, int init_capacity);
void urlpath_dispose(URLPath *path);

/**
 * @brief Resets the next_pos to 0 despite the allocated capacity. This "lazily deletes" any term hashes for the URLPath's reuse.
 * 
 * @param path 
 */
void urlpath_reset(URLPath *path);

bool urlpath_append(URLPath *path, const char *url_term);

#endif