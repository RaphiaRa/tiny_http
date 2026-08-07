#include "th_allocator.h"
#include "th_align.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct th_default_allocator {
    th_allocator base;
} th_default_allocator;

TH_LOCAL(void*)
th_default_allocator_alloc(void* self, size_t size)
{
    (void)self;
    void* ptr = malloc(size);
    return ptr;
}

TH_LOCAL(void*)
th_default_allocator_realloc(void* self, void* ptr, size_t size)
{
    (void)self;
    return realloc(ptr, size);
}

TH_LOCAL(void)
th_default_allocator_free(void* self, void* ptr)
{
    (void)self;
    free(ptr);
}

static th_default_allocator default_allocator = {
    .base = {
        .alloc = th_default_allocator_alloc,
        .realloc = th_default_allocator_realloc,
        .free = th_default_allocator_free,
    },
};

static th_allocator* user_default_allocator = NULL;

TH_PUBLIC(th_allocator*)
th_default_allocator_get(void)
{
    if (user_default_allocator)
        return user_default_allocator;
    return &default_allocator.base;
}

TH_PUBLIC(void)
th_default_allocator_set(th_allocator* allocator)
{
    user_default_allocator = allocator;
}

/* th_arena_allocator implementation begin */

TH_LOCAL(void*)
th_arena_allocator_alloc(void* self, size_t size)
{
    th_arena_allocator* allocator = self;
    if (allocator->pos + size > allocator->size) {
        if (!allocator->allocator)
            return NULL;
        return th_allocator_alloc(allocator->allocator, size);
    }
    void* ptr = (char*)allocator->buf + allocator->pos;
    allocator->prev_pos = allocator->pos;
    allocator->pos += (size_t)TH_ALIGNAS(allocator->alignment, size);
    return ptr;
}

TH_LOCAL(void*)
th_arena_allocator_realloc(void* self, void* ptr, size_t size)
{
    th_arena_allocator* allocator = self;
    if (ptr == NULL)
        return th_arena_allocator_alloc(self, size);
    if ((char*)ptr < (char*)allocator->buf || (char*)ptr >= (char*)allocator->buf + allocator->size)
        return th_allocator_realloc(allocator->allocator, ptr, size);
    if (ptr == (char*)allocator->buf + allocator->prev_pos) {
        if (allocator->prev_pos + size > allocator->size) {
            if (!allocator->allocator)
                return NULL;
            void* newp = th_allocator_alloc(allocator->allocator, size);
            if (!newp)
                return NULL;
            memcpy(newp, ptr, allocator->pos - allocator->prev_pos);
            allocator->pos = allocator->prev_pos;
            return newp;
        }
        allocator->pos = allocator->prev_pos + size;
        return ptr;
    }
    void* newp = th_allocator_alloc(self, size);
    if (!newp)
        return NULL;
    size_t max_possible = (size_t)(((uint8_t*)allocator->buf + allocator->prev_pos) - (uint8_t*)ptr);
    memcpy(newp, ptr, max_possible);
    return newp;
}

TH_LOCAL(void)
th_arena_allocator_free(void* self, void* ptr)
{
    th_arena_allocator* allocator = self;
    if ((uint8_t*)ptr == (uint8_t*)allocator->buf + allocator->prev_pos) {
        allocator->pos = allocator->prev_pos;
        return;
    }
    if ((char*)ptr < (char*)allocator->buf || (char*)ptr >= (char*)allocator->buf + allocator->size) {
        th_allocator_free(allocator->allocator, ptr);
        return;
    }
}

TH_PRIVATE(void)
th_arena_allocator_init_with_alignment(th_arena_allocator* allocator, void* buf, size_t size, size_t alignment, th_allocator* fallback)
{
    allocator->base.alloc = th_arena_allocator_alloc;
    allocator->base.realloc = th_arena_allocator_realloc;
    allocator->base.free = th_arena_allocator_free;
    allocator->allocator = fallback;
    allocator->alignment = (uint16_t)alignment;
    void* aligned = TH_ALIGNAS(alignment, buf);
    allocator->size = size - (size_t)((uint8_t*)aligned - (uint8_t*)buf);
    allocator->buf = aligned;
    allocator->pos = 0;
    allocator->prev_pos = 0;
}

TH_PRIVATE(void)
th_arena_allocator_init(th_arena_allocator* allocator, void* buf, size_t size, th_allocator* fallback)
{
    th_arena_allocator_init_with_alignment(allocator, buf, size, TH_ALIGNOF(th_max_align), fallback);
}

/* th_arena_allocator implementation end */
