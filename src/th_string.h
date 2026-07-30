#ifndef TH_STRING_H
#define TH_STRING_H

#include "th_allocator.h"
#include "th_str.h"
#include "th_vec.h"

typedef struct th_detail_large_string {
    size_t capacity;
    size_t len;
    char* ptr;
    th_allocator* allocator;
} th_detail_large_string;

#define TH_STRING_SMALL_BUF_LEN (sizeof(char*) + sizeof(size_t) + sizeof(size_t) - 1)
#define TH_STRING_SMALL_MAX_LEN (TH_STRING_SMALL_BUF_LEN - 1)
typedef struct th_detail_small_string {
    unsigned char small : 1;
    unsigned char len : 7;
    char buf[TH_STRING_SMALL_BUF_LEN];
    th_allocator* allocator;
} th_detail_small_string;

typedef struct th_string {
    union {
        th_detail_small_string small;
        th_detail_large_string large;
    } impl;
} th_string;

TH_PRIVATE(void)
th_string_init(th_string* self, th_allocator* allocator);

TH_PRIVATE(th_err)
th_string_init_with(th_string* self, th_str str, th_allocator* allocator);

TH_PRIVATE(th_err)
th_string_set(th_string* self, th_str str);

TH_PRIVATE(th_err)
th_string_append(th_string* self, th_str str);

TH_PRIVATE(th_err)
th_string_append_cstr(th_string* self, const char* str);

TH_PRIVATE(th_err)
th_string_push_back(th_string* self, char c);

TH_PRIVATE(th_err)
th_string_resize(th_string* self, size_t new_len, char fill);

TH_PRIVATE(th_str)
th_string_view(const th_string* self);

TH_PRIVATE(char*)
th_string_at(th_string* self, size_t index);

TH_PRIVATE(const char*)
th_string_data(const th_string* self);

TH_PRIVATE(size_t)
th_string_len(const th_string* self);

TH_PRIVATE(void)
th_string_deinit(th_string* self);

TH_PRIVATE(void)
th_string_clear(th_string* self);

TH_PRIVATE(void)
th_string_to_lower(th_string* self);

TH_PRIVATE(bool)
th_string_eq(const th_string* self, th_str other);

// TH_PRIVATE(uint32_t)
// th_string_hash(const th_string* self);

TH_DEFINE_VEC(th_string_vec, th_string, th_string_deinit)

#endif
