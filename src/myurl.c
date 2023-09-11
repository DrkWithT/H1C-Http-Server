/**
 * @file myurl.c
 * @author Derek Tan
 * @brief Implements URLPath.
 * @date 2023-09-10
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "utils/myurl.h"

/* URLObj Funcs. */

bool urlpath_init(URLPath *path, int init_capacity)
{
    int checked_capacity = (init_capacity >= URLPATH_MIN_CAPACITY) ? init_capacity : URLPATH_MIN_CAPACITY;

    size_t *hash_entries = calloc(checked_capacity, sizeof(size_t));

    if (!hash_entries)
    {
        path->entries = NULL;
        path->next_pos = 0;
        path->capacity = 0;
        return false;
    }

    path->entries = hash_entries;
    path->next_pos = 0;
    path->capacity = checked_capacity;

    return true;
}

void urlpath_dispose(URLPath *path)
{
    if (!path->entries)
        return;
    
    free(path->entries);
    path->entries = NULL;
}

void urlpath_reset(URLPath *path)
{
    path->next_pos = 0;
}

bool urlpath_append(URLPath *path, const char *url_term)
{
    bool ok = false;

    if (!path->entries)
        return ok;

    int next_pos_copy = path->next_pos;
    int old_capacity = path->capacity;
    int new_capacity = old_capacity << 1;
    size_t term_hash = hash_cstr(url_term);

    if (next_pos_copy < old_capacity)
    {
        path->entries[next_pos_copy] = term_hash;
        path->next_pos++;
        ok = true;

        return ok;
    }

    size_t *new_entry_ptr = realloc(path->entries, new_capacity * sizeof(size_t));
    ok = new_entry_ptr != NULL;

    if (ok)
    {
        memset(new_entry_ptr + next_pos_copy, 0, new_capacity - next_pos_copy);

        path->entries = new_entry_ptr;
        path->next_pos++;
        path->capacity = new_capacity;
    }

    return ok;
}
