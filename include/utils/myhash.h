#ifndef MYHASH_H
#define MYHASH_H

#include <stdint.h>
#include <string.h>

#define MY_HASH_PRIME 3

size_t hash_cstr(const char *str);

#endif