#ifndef TH_ALLOCATOR_H
#define TH_ALLOCATOR_H

#include <stddef.h>

#include "th_config.h"
#include "th_list.h"
#include "th_utility.h"

TH_INLINE(void*)
th_allocator_alloc(th_allocator* allocator, size_t size)
{
    return allocator->alloc(allocator, size);
}

TH_INLINE(void*)
th_allocator_realloc(th_allocator* allocator, void* ptr, size_t size)
{
    return allocator->realloc(allocator, ptr, size);
}

TH_INLINE(void)
th_allocator_free(th_allocator* allocator, void* ptr)
{
    allocator->free(allocator, ptr);
}

TH_PRIVATE(th_allocator*)
th_default_allocator_get(void);

/* th_arena_allocator begin */

typedef struct th_arena_allocator {
    th_allocator base;
    th_allocator* allocator;
    void* buf;
    size_t size;
    size_t pos;
    size_t prev_pos;
    uint16_t alignment;
} th_arena_allocator;

/** th_arena_allocator_init
 * @brief The arena allocator is a simple allocator that allocates memory from a fixed-size buffer.
 * It only frees memory when the free operation is called on the previously allocated memory.
 * If no memory is available in the buffer, it will fall back to the default allocator.
 * @param allocator The arena allocator to initialize.
 * @param buf The buffer to use for allocations.
 * @param size The size of the buffer.
 */
TH_PRIVATE(void)
th_arena_allocator_init(th_arena_allocator* allocator, void* buf, size_t size, th_allocator* fallback);

/** th_arena_allocator_init_with_alignment
 * @brief Just like th_arena_allocator_init, but allows specifying the alignment of the allocations.
 */
TH_PRIVATE(void)
th_arena_allocator_init_with_alignment(th_arena_allocator* allocator, void* buf, size_t size, size_t alignment, th_allocator* fallback);

/* th_arena_allocator end */
/* th_pool_allocator begin */
typedef struct th_pool_allocator_node th_pool_allocator_node;
struct th_pool_allocator_node {
    th_pool_allocator_node* next;
    th_pool_allocator_node* prev;
};
TH_DEFINE_LIST(th_pool_allocator_list, th_pool_allocator_node, prev, next)
typedef struct th_pool_allocator {
    th_allocator base;
    th_pool_allocator_list free_list;
    th_pool_allocator_list used_list;
    th_allocator* allocator;
    size_t block_size;
} th_pool_allocator;

TH_PRIVATE(void)
th_pool_allocator_init(th_pool_allocator* pool, th_allocator* allocator, size_t block_size);

TH_PRIVATE(void)
th_pool_allocator_deinit(th_pool_allocator* pool);

/** Generic object pool allocator.
 * The pool allocator is a allocator that allocates objects from a pool of fixed-size blocks.
 * It can be used with any object that has a next and prev pointer.
 */
#define TH_DEFINE_OBJ_POOL_ALLOCATOR(NAME, T, PREV, NEXT)                                         \
    TH_DEFINE_LIST(NAME##_list, T, PREV, NEXT)                                                    \
    typedef struct NAME {                                                                         \
        th_allocator base;                                                                        \
        NAME##_list free_list;                                                                    \
        NAME##_list used_list;                                                                    \
        th_allocator* allocator;                                                                  \
        size_t count;                                                                             \
        size_t max;                                                                               \
    } NAME;                                                                                       \
                                                                                                  \
    TH_INLINE(void)                                                                               \
    NAME##_init(NAME* pool, th_allocator* allocator, size_t initial, size_t max) TH_MAYBE_UNUSED; \
                                                                                                  \
    TH_INLINE(void)                                                                               \
    NAME##_deinit(NAME* pool) TH_MAYBE_UNUSED;                                                    \
                                                                                                  \
    TH_INLINE(void*)                                                                              \
    NAME##_alloc(void* self, size_t) TH_MAYBE_UNUSED;                                             \
                                                                                                  \
    TH_INLINE(void)                                                                               \
    NAME##_free(void* self, void* ptr) TH_MAYBE_UNUSED;                                           \
                                                                                                  \
    TH_INLINE(void)                                                                               \
    NAME##_init(NAME* pool, th_allocator* allocator, size_t initial, size_t max)                  \
    {                                                                                             \
        TH_ASSERT(allocator != NULL && "Invalid allocator");                                      \
        TH_ASSERT(max > 0 && "Invalid max");                                                      \
        pool->base.alloc = NAME##_alloc;                                                          \
        pool->base.realloc = NULL;                                                                \
        pool->base.free = NAME##_free;                                                            \
        pool->allocator = allocator;                                                              \
        pool->count = 0;                                                                          \
        pool->max = max;                                                                          \
        pool->used_list = (NAME##_list){0};                                                       \
        pool->free_list = (NAME##_list){0};                                                       \
        for (size_t i = 0; i < initial; i++) {                                                    \
            T* item = (T*)th_allocator_alloc(pool->allocator, sizeof(T));                         \
            if (item) {                                                                           \
                NAME##_list_push_back(&pool->free_list, item);                                    \
                ++pool->count;                                                                    \
            }                                                                                     \
        }                                                                                         \
    }                                                                                             \
                                                                                                  \
    TH_INLINE(void)                                                                               \
    NAME##_deinit(NAME* pool)                                                                     \
    {                                                                                             \
        T* item = NULL;                                                                           \
        while ((item = NAME##_list_pop_front(&pool->free_list))) {                                \
            th_allocator_free(pool->allocator, item);                                             \
        }                                                                                         \
        item = NAME##_list_pop_front(&pool->used_list);                                           \
        TH_ASSERT(item == NULL);                                                                  \
    }                                                                                             \
                                                                                                  \
    TH_INLINE(void*)                                                                              \
    NAME##_alloc(void* self, size_t size)                                                         \
    {                                                                                             \
        TH_ASSERT(size == sizeof(T) && "Invalid size");                                           \
        (void)size;                                                                               \
        NAME* pool = (NAME*)self;                                                                 \
        T* item = NAME##_list_pop_front(&pool->free_list);                                        \
        if (item == NULL) {                                                                       \
            if (pool->count < pool->max) {                                                        \
                item = (T*)th_allocator_alloc(pool->allocator, sizeof(T));                        \
                if (item) {                                                                       \
                    pool->count++;                                                                \
                }                                                                                 \
            }                                                                                     \
        }                                                                                         \
        if (item) {                                                                               \
            NAME##_list_push_back(&pool->used_list, item);                                        \
        }                                                                                         \
        return item;                                                                              \
    }                                                                                             \
                                                                                                  \
    TH_INLINE(void)                                                                               \
    NAME##_free(void* self, void* ptr)                                                            \
    {                                                                                             \
        NAME* pool = (NAME*)self;                                                                 \
        T* item = (T*)ptr;                                                                        \
        if (item) {                                                                               \
            NAME##_list_erase(&pool->used_list, item);                                            \
            NAME##_list_push_back(&pool->free_list, item);                                        \
        }                                                                                         \
    }

#endif
