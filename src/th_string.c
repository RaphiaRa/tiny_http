#include "th_string.h"
#include "th_align.h"
#include "th_hash.h"
#include "th_utility.h"

#include <ctype.h>

#define TH_STRING_SMALL (sizeof(char*) + sizeof(size_t) + sizeof(size_t) - 2)
#define TH_STRING_ALIGNUP(size) TH_ALIGNUP(size, 16)
TH_LOCAL(void)
th_detail_small_string_init(th_detail_small_string* self, th_allocator* allocator)
{
    self->small = 1;
    self->len = 0;
    self->buf[0] = '\0';
    self->allocator = allocator;
    if (self->allocator == NULL) {
        self->allocator = th_default_allocator_get();
    }
}

TH_PRIVATE(void)
th_string_init(th_string* self, th_allocator* allocator)
{
    th_detail_small_string_init(&self->impl.small, allocator);
}

TH_PRIVATE(th_err)
th_string_init_with(th_string* self, th_str str, th_allocator* allocator)
{
    th_string_init(self, allocator);
    return th_string_set(self, str);
}

TH_LOCAL(void)
th_detail_small_string_set(th_detail_small_string* self, th_str str)
{
    TH_ASSERT(str.len <= TH_STRING_SMALL_MAX_LEN);
    if (str.len > 0)
        memcpy(self->buf, str.ptr, str.len);
    self->buf[str.len] = '\0';
    self->len = str.len & 0x7F;
}

TH_LOCAL(th_err)
th_detail_large_string_set(th_detail_large_string* self, th_str str)
{
    size_t required_capacity = str.len + 1;
    if (self->capacity < required_capacity) {
        size_t new_capacity = TH_STRING_ALIGNUP(required_capacity);
        char* new_ptr = th_allocator_realloc(self->allocator, self->ptr, new_capacity);
        if (new_ptr == NULL) {
            return TH_ERR_BAD_ALLOC;
        }
        self->ptr = new_ptr;
        self->capacity = new_capacity;
    }
    self->len = str.len;
    if (str.len > 0)
        memcpy(self->ptr, str.ptr, str.len);
    self->ptr[str.len] = '\0';
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_string_small_to_large(th_string* self, size_t capacity)
{
    TH_ASSERT(self->impl.small.small);
    th_detail_large_string large = {0};
    capacity = TH_STRING_ALIGNUP(capacity);
    large.capacity = capacity;
    large.len = self->impl.small.len;
    large.ptr = th_allocator_alloc(self->impl.small.allocator, capacity);
    if (large.ptr == NULL) {
        return TH_ERR_BAD_ALLOC;
    }
    large.allocator = self->impl.small.allocator;
    memcpy(large.ptr, self->impl.small.buf, self->impl.small.len);
    large.ptr[self->impl.small.len] = '\0';
    self->impl.large = large;
    return TH_ERR_OK;
}

TH_PRIVATE(th_err)
th_string_set(th_string* self, th_str str)
{
    TH_ASSERT(str.ptr != NULL && "Invalid string");
    if (self->impl.small.small) {
        if (str.len <= TH_STRING_SMALL_MAX_LEN) {
            th_detail_small_string_set(&self->impl.small, str);
            return TH_ERR_OK;
        } else {
            th_err err = th_string_small_to_large(self, str.len + 1);
            if (err != TH_ERR_OK)
                return err;
        }
    }
    return th_detail_large_string_set(&self->impl.large, str);
}

TH_LOCAL(void)
th_detail_small_string_append(th_detail_small_string* self, th_str str)
{
    TH_ASSERT(self->len + str.len <= TH_STRING_SMALL_MAX_LEN);
    memcpy(self->buf + self->len, str.ptr, str.len);
    self->len += str.len & 0x7F;
    self->buf[self->len] = '\0';
}

TH_LOCAL(th_err)
th_detail_large_string_append(th_detail_large_string* self, th_str str)
{
    size_t required_capacity = self->len + str.len + 1;
    if (required_capacity > self->capacity) {
        size_t new_capacity = TH_STRING_ALIGNUP(required_capacity);
        char* new_ptr = th_allocator_realloc(self->allocator, self->ptr, new_capacity);
        if (new_ptr == NULL) {
            return TH_ERR_BAD_ALLOC;
        }
        self->ptr = new_ptr;
        self->capacity = new_capacity;
    }
    memcpy(self->ptr + self->len, str.ptr, str.len);
    self->len += str.len;
    self->ptr[self->len] = '\0';
    return TH_ERR_OK;
}

TH_PRIVATE(th_err)
th_string_append(th_string* self, th_str str)
{
    if (self->impl.small.small) {
        if (self->impl.small.len + str.len <= TH_STRING_SMALL_MAX_LEN) {
            th_detail_small_string_append(&self->impl.small, str);
            return TH_ERR_OK;
        } else {
            th_err err = th_string_small_to_large(self, self->impl.small.len + str.len + 1);
            if (err != TH_ERR_OK)
                return err;
        }
    }
    return th_detail_large_string_append(&self->impl.large, str);
}

TH_PRIVATE(th_err)
th_string_append_cstr(th_string* self, const char* str)
{
    return th_string_append(self, th_str_make(str, strlen(str)));
}

TH_PRIVATE(th_err)
th_string_push_back(th_string* self, char c)
{
    return th_string_append(self, (th_str){&c, 1});
}

TH_LOCAL(void)
th_detail_small_string_resize(th_detail_small_string* self, size_t new_len, char fill)
{
    TH_ASSERT(new_len <= TH_STRING_SMALL_MAX_LEN && "Invalid length");
    if (new_len > self->len)
        memset(self->buf + self->len, fill, new_len - self->len);
    self->len = new_len & 0x7F;
    self->buf[new_len] = '\0';
}

TH_LOCAL(th_err)
th_detail_large_string_resize(th_detail_large_string* self, size_t new_len, char fill)
{
    size_t required_capacity = new_len + 1;
    if (required_capacity > self->capacity) {
        size_t new_capacity = TH_STRING_ALIGNUP(required_capacity);
        char* new_ptr = th_allocator_realloc(self->allocator, self->ptr, new_capacity);
        if (new_ptr == NULL) {
            return TH_ERR_BAD_ALLOC;
        }
        self->ptr = new_ptr;
        self->capacity = new_capacity;
    }
    if (new_len > self->len)
        memset(self->ptr + self->len, fill, new_len - self->len);
    self->len = new_len;
    self->ptr[new_len] = '\0';
    return TH_ERR_OK;
}

TH_PRIVATE(th_err)
th_string_resize(th_string* self, size_t new_len, char fill)
{
    if (self->impl.small.small) {
        if (new_len <= TH_STRING_SMALL_MAX_LEN) {
            th_detail_small_string_resize(&self->impl.small, new_len, fill);
            return TH_ERR_OK;
        } else {
            th_err err = th_string_small_to_large(self, new_len + 1);
            if (err != TH_ERR_OK)
                return err;
        }
    }
    return th_detail_large_string_resize(&self->impl.large, new_len, fill);
}

TH_PRIVATE(th_str)
th_string_view(const th_string* self)
{
    if (self->impl.small.small) {
        return (th_str){self->impl.small.buf, self->impl.small.len};
    } else {
        return (th_str){self->impl.large.ptr, self->impl.large.len};
    }
}

TH_PRIVATE(const char*)
th_string_data(const th_string* self)
{
    if (self->impl.small.small) {
        return self->impl.small.buf;
    } else {
        return self->impl.large.ptr;
    }
}

TH_PRIVATE(char*)
th_string_at(th_string* self, size_t index)
{
    TH_ASSERT(index < th_string_len(self) && "Index out of bounds");
    if (self->impl.small.small) {
        return &self->impl.small.buf[index];
    } else {
        return &self->impl.large.ptr[index];
    }
}

TH_PRIVATE(size_t)
th_string_len(const th_string* self)
{
    if (self->impl.small.small) {
        return self->impl.small.len;
    } else {
        return self->impl.large.len;
    }
}

TH_PRIVATE(void)
th_string_clear(th_string* self)
{
    if (self->impl.small.small) {
        self->impl.small.len = 0;
        self->impl.small.buf[0] = '\0';
    } else {
        self->impl.large.len = 0;
        self->impl.large.ptr[0] = '\0';
    }
}

TH_PRIVATE(void)
th_string_to_lower(th_string* self)
{
    char* ptr = th_string_at(self, 0);
    size_t n = th_string_len(self);
    for (size_t i = 0; i < n; i++) {
        ptr[i] = (char)tolower((int)ptr[i]);
    }
}

TH_PRIVATE(bool)
th_string_eq(const th_string* self, th_str other)
{
    const char* ptr = NULL;
    size_t n = 0;
    if (self->impl.small.small) {
        ptr = self->impl.small.buf;
        n = self->impl.small.len;
    } else {
        ptr = self->impl.large.ptr;
        n = self->impl.large.len;
    }
    return n == other.len && (n == 0 || memcmp(ptr, other.ptr, n) == 0);
}

// TH_PRIVATE(uint32_t)
// th_string_hash(const th_string* self)
//{
//     const char* ptr = NULL;
//     size_t n = 0;
//     if (self->impl.small.small) {
//         ptr = self->impl.small.buf;
//         n = self->impl.small.len;
//     } else {
//         ptr = self->impl.large.ptr;
//         n = self->impl.large.len;
//     }
//     return th_hash_bytes(ptr, n);
// }

TH_PRIVATE(void)
th_string_deinit(th_string* self)
{
    if (!self->impl.small.small) {
        th_allocator_free(self->impl.large.allocator, self->impl.large.ptr);
    }
}
