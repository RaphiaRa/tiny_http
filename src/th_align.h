#ifndef TH_ALIGN_H
#define TH_ALIGN_H

#include <stdint.h>

#define TH_ALIGNOF(type) ((size_t)&(((struct { char c; type member; }*)0)->member))
#define TH_ALIGNAS(align, ptr) ((void*)(((uintptr_t)(ptr) + ((align) - 1)) & ~((align) - 1)))
#define TH_ALIGNUP(n, align) (((n) + (size_t)(align) - 1) & ~((size_t)(align) - 1))
#define TH_ALIGNDOWN(n, align) ((n) & ~((align) - 1))

typedef long double th_max_align;

#endif
