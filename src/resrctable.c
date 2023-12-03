/**
 * @file resrctable.c
 * @author Derek Tan
 * @brief Implements resource hashtable.
 * @date 2023-09-13
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "utils/resrctable.h"

bool restable_init(ResourceTable *restable, uint16_t count)
{
    // Check count before converting to capacity since unsigned shorts are easier to overflow. Invalid sizes would mess up the allocations.
    uint16_t temp_capacity = (count >= RESTABLE_DEFAULT_COUNT && count < RESTABLE_MAX_COUNT)
        ? count
        : RESTABLE_DEFAULT_COUNT;

    temp_capacity <<= 1;  // I double this capacity from the count since a 0.5 load factor really reduces collisions.

    StaticResource **ref_buckets = malloc(temp_capacity * sizeof(StaticResource *));
    bool init_ok = ref_buckets != NULL;

    if (init_ok)
    {
        for (uint16_t i = 0; i < temp_capacity; i++)
            ref_buckets[i] = NULL;
        
        restable->resources = ref_buckets;
        restable->capacity = temp_capacity;
    }
    else
    {
        restable->resources = NULL;
        restable->capacity = 0;
    }

    return init_ok;
}

void restable_dispose(ResourceTable *restable)
{
    uint16_t bucket_count = restable->capacity;

    if (bucket_count == 0 || !restable->resources)
        return;

    uint16_t bucket_pos = 0;
    StaticResource **statres_ref = restable->resources;
    StaticResource *curr_ref = NULL;

    while (bucket_pos < bucket_count)
    {
        curr_ref = *statres_ref;

        if (!curr_ref)
            continue;

        statsrc_dispose(curr_ref);
        free(curr_ref);

        statres_ref++;
    }
    
    free(restable->resources);
    restable->resources = NULL;
}

bool restable_put(ResourceTable *restable, const char *key, StaticResource *resrc)
{
    uint16_t bucket_pos = hash_cstr(key) % restable->capacity;

    StaticResource *bucket_ptr = restable->resources[bucket_pos];

    // Only fill empty buckets to not handle collisions- The table capacity will always have a 0.5 max load factor.
    if (!bucket_ptr)
        restable->resources[bucket_pos] = resrc;
    
    return bucket_ptr == NULL;
}

const StaticResource *restable_get(const ResourceTable *restable, const char *key)
{
    uint16_t bucket_pos = hash_cstr(key) % restable->capacity;

    return restable->resources[bucket_pos];
}
