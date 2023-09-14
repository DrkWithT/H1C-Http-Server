#ifndef RESRCTABLE_H
#define RESRCTABLE_H

#include <stdbool.h>
#include <stdlib.h>
#include "utils/myhash.h"
#include "utils/resource.h"

/* Magic Macros */

/**
 * @brief The default, fixed hashtable size for the static resource table. Assumes a small number of served resources (~10 for minimal collisions).
 */
#define RESTABLE_DEFAULT_COUNT 8

#define RESTABLE_MAX_COUNT (UINT16_MAX >> 1)

/* ResourceTable Struct */

typedef struct restable_t
{
    uint16_t capacity;           // total bucket count
    StaticResource **resources;  // entries
} ResourceTable;

/* ResourceTable Funcs. */

bool restable_init(ResourceTable *restable, uint16_t count);
void restable_dispose(ResourceTable *restable);
bool restable_put(ResourceTable *restable, const char *key, StaticResource *resrc);
const StaticResource *restable_get(const ResourceTable *restable, const char *key);

#endif