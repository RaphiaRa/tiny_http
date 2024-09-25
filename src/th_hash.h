#ifndef TH_HASH_H
#define TH_HASH_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "th_config.h"

/** th_hash_bytes
 * @brief Fowler-Noll-Vo hash function (FNV-1a).
 * See https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
 */
TH_INLINE(uint32_t)
th_hash_bytes(const void* data, size_t len)
{
    uint32_t hash = 2166136261u;
    const uint8_t* bytes = (const uint8_t*)data;
    for (size_t i = 0; i < len; ++i) {
        hash ^= bytes[i];
        hash *= 16777619;
    }
    return hash;
}

TH_INLINE(uint32_t)
th_hash_cstr(const char* str)
{
    return th_hash_bytes(str, strlen(str));
}

#endif
