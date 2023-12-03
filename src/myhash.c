/**
 * @file myhash.c
 * @author Derek Tan
 * @brief Implements a simple hash function.
 * @date 2023-09-10
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "utils/myhash.h"

size_t hash_cstr(const char *str)
{
    size_t len = strnlen(str, MY_HASH_LIMIT);
    size_t base = 1;
    size_t hash = 0;

    if (!str)
        return hash;

    for (size_t i = 0; i < len; i++)
    {
        hash += base * str[i];
        base *= MY_HASH_PRIME;
    }

    return hash;
}
