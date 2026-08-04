#include <th.h>

#include <stdbool.h>
#include <string.h>

#include "th_config.h"
#include "th_hash.h"
#include "th_str.h"

size_t th_str_npos = (size_t)-1;

TH_PRIVATE(bool)
th_str_is_uint(th_str str)
{
    for (size_t i = 0; i < str.len; i++) {
        if (str.ptr[i] < '0' || str.ptr[i] > '9') {
            return false;
        }
    }
    return true;
}

TH_PRIVATE(th_err)
th_str_to_uint(th_str str, unsigned int* out)
{
    *out = 0;
    for (size_t i = 0; i < str.len; i++) {
        if (str.ptr[i] < '0' || str.ptr[i] > '9')
            return TH_ERR_INVALID_ARG;
        *out = *out * 10 + (unsigned int)(str.ptr[i] - '0');
    }
    return TH_ERR_OK;
}

TH_PRIVATE(bool)
th_str_eq(th_str a, th_str b)
{
    if (a.len != b.len) {
        return 0;
    }
    return memcmp(a.ptr, b.ptr, a.len) == 0;
}

TH_PRIVATE(size_t)
th_str_find_first(th_str str, size_t start, char c)
{
    if (start >= str.len) {
        return th_str_npos;
    }
    const char* found = memchr(str.ptr + start, c, str.len - start);
    return found ? (size_t)(found - str.ptr) : th_str_npos;
}

TH_PRIVATE(size_t)
th_str_find_first_not(th_str str, size_t start, char c)
{
    for (size_t i = start; i < str.len; i++) {
        if (str.ptr[i] != c) {
            return i;
        }
    }
    return th_str_npos;
}

TH_PRIVATE(size_t)
th_str_find_first_of(th_str str, size_t start, const char* chars)
{
    size_t chars_len = strlen(chars);
    for (size_t i = start; i < str.len; i++) {
        for (size_t j = 0; j < chars_len; j++) {
            if (str.ptr[i] == chars[j]) {
                return i;
            }
        }
    }
    return th_str_npos;
}

TH_PRIVATE(size_t)
th_str_find_last(th_str str, size_t start, char c)
{
    for (size_t i = start; i < str.len; i++) {
        if (str.ptr[str.len - i - 1] == c) {
            return i;
        }
    }
    return th_str_npos;
}

TH_PRIVATE(th_str)
th_str_substr(th_str str, size_t start, size_t len)
{
    if (start >= str.len) {
        return th_str_make(str.ptr + len, 0);
    }
    if (len == th_str_npos || start + len > str.len) {
        len = str.len - start;
    }
    return th_str_make(str.ptr + start, len);
}

TH_PRIVATE(th_str)
th_str_trim(th_str str)
{
    size_t start = 0;
    while (start < str.len && (str.ptr[start] == ' ' || str.ptr[start] == '\t')) {
        start++;
    }
    size_t end = str.len;
    while (end > start && (str.ptr[end - 1] == ' ' || str.ptr[end - 1] == '\t')) {
        end--;
    }
    return th_str_substr(str, start, end - start);
}

TH_PRIVATE(size_t)
th_str_hash(th_str str)
{
    return th_hash_bytes(str.ptr, str.len);
}
