#ifndef TH_HEAP_STRING_H
#define TH_HEAP_STRING_H

#include "th_allocator.h"
#include "th_string.h"
#include "th_vec.h"

typedef struct th_detail_large_string {
    size_t capacity;
    size_t len;
    char* ptr;
    th_allocator* allocator;
} th_detail_large_string;

#define TH_HEAP_STRING_SMALL_BUF_LEN (sizeof(char*) + sizeof(size_t) + sizeof(size_t) - 1)
#define TH_HEAP_STRING_SMALL_MAX_LEN (TH_HEAP_STRING_SMALL_BUF_LEN - 1)
typedef struct th_detail_small_string {
    unsigned char small : 1;
    unsigned char len : 7;
    char buf[TH_HEAP_STRING_SMALL_BUF_LEN];
    th_allocator* allocator;
} th_detail_small_string;

typedef struct th_heap_string {
    union {
        th_detail_small_string small;
        th_detail_large_string large;
    } impl;
} th_heap_string;

TH_PRIVATE(void)
th_heap_string_init(th_heap_string* self, th_allocator* allocator);

TH_PRIVATE(th_err)
th_heap_string_init_with(th_heap_string* self, th_string str, th_allocator* allocator);

TH_PRIVATE(th_err)
th_heap_string_set(th_heap_string* self, th_string str);

TH_PRIVATE(th_err)
th_heap_string_append(th_heap_string* self, th_string str);

TH_PRIVATE(th_err)
th_heap_string_push_back(th_heap_string* self, char c);

TH_PRIVATE(th_err)
th_heap_string_resize(th_heap_string* self, size_t new_len, char fill);

TH_PRIVATE(th_string)
th_heap_string_view(const th_heap_string* self);

TH_PRIVATE(char*)
th_heap_string_at(th_heap_string* self, size_t index);

TH_PRIVATE(const char*)
th_heap_string_data(const th_heap_string* self);

TH_PRIVATE(size_t)
th_heap_string_len(const th_heap_string* self);

TH_PRIVATE(void)
th_heap_string_deinit(th_heap_string* self);

TH_PRIVATE(void)
th_heap_string_clear(th_heap_string* self);

TH_PRIVATE(void)
th_heap_string_to_lower(th_heap_string* self);

TH_PRIVATE(bool)
th_heap_string_eq(const th_heap_string* self, th_string other);

TH_PRIVATE(uint32_t)
th_heap_string_hash(const th_heap_string* self);

TH_DEFINE_VEC(th_heap_string_vec, th_heap_string, th_heap_string_deinit)

#endif
