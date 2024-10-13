#ifndef TH_VEC_H
#define TH_VEC_H

#include <th.h>

#include "th_allocator.h"
#include "th_config.h"

#include <stddef.h>

#define TH_DEFINE_VEC(NAME, TYPE, DEINIT)                                                                  \
    typedef struct NAME {                                                                                  \
        TYPE* data;                                                                                        \
        size_t size;                                                                                       \
        size_t capacity;                                                                                   \
        th_allocator* allocator;                                                                           \
    } NAME;                                                                                                \
                                                                                                           \
    TH_INLINE(void)                                                                                        \
    NAME##_init(NAME* vec, th_allocator* allocator) TH_MAYBE_UNUSED;                                       \
                                                                                                           \
    TH_INLINE(void)                                                                                        \
    NAME##_clear(NAME* vec) TH_MAYBE_UNUSED;                                                               \
                                                                                                           \
    TH_INLINE(void)                                                                                        \
    NAME##_deinit(NAME* vec) TH_MAYBE_UNUSED;                                                              \
                                                                                                           \
    TH_INLINE(size_t)                                                                                      \
    NAME##_size(NAME* vec) TH_MAYBE_UNUSED;                                                                \
                                                                                                           \
    TH_INLINE(size_t)                                                                                      \
    NAME##_capacity(NAME* vec) TH_MAYBE_UNUSED;                                                            \
                                                                                                           \
    TH_INLINE(th_err)                                                                                      \
    NAME##_resize(NAME* vec, size_t size) TH_MAYBE_UNUSED;                                                 \
                                                                                                           \
    TH_INLINE(th_err)                                                                                      \
    NAME##_push_back(NAME* vec, TYPE value) TH_MAYBE_UNUSED;                                               \
                                                                                                           \
    TH_INLINE(TYPE*)                                                                                       \
    NAME##_at(NAME* vec, size_t index) TH_MAYBE_UNUSED;                                                    \
                                                                                                           \
    TH_INLINE(TYPE*)                                                                                       \
    NAME##_begin(NAME* vec) TH_MAYBE_UNUSED;                                                               \
                                                                                                           \
    TH_INLINE(TYPE*)                                                                                       \
    NAME##_end(NAME* vec) TH_MAYBE_UNUSED;                                                                 \
                                                                                                           \
    TH_INLINE(void)                                                                                        \
    NAME##_init(NAME* vec, th_allocator* allocator)                                                        \
    {                                                                                                      \
        vec->allocator = allocator ? allocator : th_default_allocator_get();                               \
        vec->capacity = 0;                                                                                 \
        vec->size = 0;                                                                                     \
        vec->data = NULL;                                                                                  \
    }                                                                                                      \
                                                                                                           \
    TH_INLINE(void)                                                                                        \
    NAME##_deinit(NAME* vec)                                                                               \
    {                                                                                                      \
        if (vec->data) {                                                                                   \
            for (size_t i = 0; i < vec->size; i++) {                                                       \
                DEINIT(&vec->data[i]);                                                                     \
            }                                                                                              \
            th_allocator_free(vec->allocator, vec->data);                                                  \
        }                                                                                                  \
    }                                                                                                      \
                                                                                                           \
    TH_INLINE(void)                                                                                        \
    NAME##_clear(NAME* vec)                                                                                \
    {                                                                                                      \
        if (vec->data) {                                                                                   \
            for (size_t i = 0; i < vec->size; i++) {                                                       \
                DEINIT(&vec->data[i]);                                                                     \
            }                                                                                              \
        }                                                                                                  \
        vec->size = 0;                                                                                     \
    }                                                                                                      \
                                                                                                           \
    TH_INLINE(size_t)                                                                                      \
    NAME##_size(NAME* vec)                                                                                 \
    {                                                                                                      \
        return vec->size;                                                                                  \
    }                                                                                                      \
                                                                                                           \
    TH_INLINE(size_t)                                                                                      \
    NAME##_capacity(NAME* vec)                                                                             \
    {                                                                                                      \
        return vec->capacity;                                                                              \
    }                                                                                                      \
                                                                                                           \
    TH_INLINE(th_err)                                                                                      \
    NAME##_resize(NAME* vec, size_t size)                                                                  \
    {                                                                                                      \
        if (size < vec->size) {                                                                            \
            vec->size = size;                                                                              \
            return TH_ERR_OK;                                                                              \
        }                                                                                                  \
        if (size > vec->capacity) {                                                                        \
            size_t new_capacity = th_next_pow2(size);                                                      \
            TYPE* new_data = th_allocator_realloc(vec->allocator, vec->data, new_capacity * sizeof(TYPE)); \
            if (new_data == NULL) {                                                                        \
                return TH_ERR_BAD_ALLOC;                                                                   \
            }                                                                                              \
            vec->data = new_data;                                                                          \
            vec->capacity = new_capacity;                                                                  \
        }                                                                                                  \
        vec->size = size;                                                                                  \
        return TH_ERR_OK;                                                                                  \
    }                                                                                                      \
                                                                                                           \
    TH_INLINE(th_err)                                                                                      \
    NAME##_push_back(NAME* vec, TYPE value)                                                                \
    {                                                                                                      \
        if (vec->size >= vec->capacity) {                                                                  \
            size_t new_capacity = vec->capacity == 0 ? 1 : vec->capacity * 2;                              \
            TYPE* new_data = th_allocator_realloc(vec->allocator, vec->data, new_capacity * sizeof(TYPE)); \
            if (new_data == NULL) {                                                                        \
                return TH_ERR_BAD_ALLOC;                                                                   \
            }                                                                                              \
            vec->data = new_data;                                                                          \
            vec->capacity = new_capacity;                                                                  \
        }                                                                                                  \
        vec->data[vec->size++] = value;                                                                    \
        return TH_ERR_OK;                                                                                  \
    }                                                                                                      \
                                                                                                           \
    TH_INLINE(TYPE*)                                                                                       \
    NAME##_at(NAME* vec, size_t index)                                                                     \
    {                                                                                                      \
        TH_ASSERT(index <= vec->size);                                                                     \
        return vec->data + index;                                                                          \
    }                                                                                                      \
                                                                                                           \
    TH_INLINE(TYPE*)                                                                                       \
    NAME##_begin(NAME* vec)                                                                                \
    {                                                                                                      \
        return vec->data;                                                                                  \
    }                                                                                                      \
                                                                                                           \
    TH_INLINE(TYPE*)                                                                                       \
    NAME##_end(NAME* vec)                                                                                  \
    {                                                                                                      \
        return vec->data + vec->size;                                                                      \
    }

// Default vectors
TH_DEFINE_VEC(th_buf_vec, char, (void))

#endif
