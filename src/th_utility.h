#ifndef TH_UTILITY_H
#define TH_UTILITY_H

#include "th_log.h"

#include <stdlib.h>

#define TH_MIN(a, b) ((a) < (b) ? (a) : (b))
#define TH_MAX(a, b) ((a) > (b) ? (a) : (b))
#define TH_ABS(a) ((a) < 0 ? -(a) : (a))

#define TH_ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

// Move a pointer from src to dst and set src to NULL
TH_INLINE(void*)
th_move_ptr(void** src)
{
    void* dst = *src;
    *src = NULL;
    return dst;
}

#define TH_MOVE_PTR(ptr) th_move_ptr((void**)&(ptr))

// Custom assert macros

#ifndef NDEBUG
#define TH_ASSERT(cond)                                                               \
    do {                                                                              \
        if (!(cond)) {                                                                \
            TH_LOG_FATAL("Assertion failed: %s at %s:%d", #cond, __FILE__, __LINE__); \
            abort();                                                                  \
        }                                                                             \
    } while (0)
#else
#define TH_ASSERT(cond) ((void)0)
#endif

// Mathematical utility functions

TH_INLINE(size_t)
th_next_pow2(size_t n)
{
    TH_ASSERT(n > 0);
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;
    return n;
}

#endif
