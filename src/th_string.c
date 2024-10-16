#include <th.h>

#include <stdbool.h>

#include "th_config.h"
#include "th_hash.h"
#include "th_string.h"

size_t th_string_npos = (size_t)-1;

TH_PRIVATE(bool)
th_string_is_uint(th_string str)
{
    for (size_t i = 0; i < str.len; i++) {
        if (str.ptr[i] < '0' || str.ptr[i] > '9') {
            return false;
        }
    }
    return true;
}

TH_PRIVATE(th_err)
th_string_to_uint(th_string str, unsigned int* out)
{
    *out = 0;
    for (size_t i = 0; i < str.len; i++) {
        if (str.ptr[i] < '0' || str.ptr[i] > '9')
            return TH_ERR_INVALID_ARG;
        *out = *out * 10 + (str.ptr[i] - '0');
    }
    return TH_ERR_OK;
}

TH_PRIVATE(bool)
th_string_eq(th_string a, th_string b)
{
    if (a.len != b.len) {
        return 0;
    }
    for (size_t i = 0; i < a.len; i++) {
        if (a.ptr[i] != b.ptr[i]) {
            return 0;
        }
    }
    return 1;
}

TH_PRIVATE(size_t)
th_string_find_first(th_string str, size_t start, char c)
{
    for (size_t i = start; i < str.len; i++) {
        if (str.ptr[i] == c) {
            return i;
        }
    }
    return th_string_npos;
}

TH_PRIVATE(size_t)
th_string_find_first_not(th_string str, size_t start, char c)
{
    for (size_t i = start; i < str.len; i++) {
        if (str.ptr[i] != c) {
            return i;
        }
    }
    return th_string_npos;
}

TH_PRIVATE(size_t)
th_string_find_first_of(th_string str, size_t start, const char* chars)
{
    for (size_t i = start; i < str.len; i++) {
        for (size_t j = 0; chars[j] != '\0'; j++) {
            if (str.ptr[i] == chars[j]) {
                return i;
            }
        }
    }
    return th_string_npos;
}

TH_PRIVATE(size_t)
th_string_find_last(th_string str, size_t start, char c)
{
    for (size_t i = start; i < str.len; i++) {
        if (str.ptr[str.len - i - 1] == c) {
            return i;
        }
    }
    return th_string_npos;
}

TH_PRIVATE(th_string)
th_string_substr(th_string str, size_t start, size_t len)
{
    if (start >= str.len) {
        return th_string_make(str.ptr + len, 0);
    }
    if (len == th_string_npos || start + len > str.len) {
        len = str.len - start;
    }
    return th_string_make(str.ptr + start, len);
}

TH_PRIVATE(th_string)
th_string_trim(th_string str)
{
    size_t start = 0;
    while (start < str.len && (str.ptr[start] == ' ' || str.ptr[start] == '\t')) {
        start++;
    }
    size_t end = str.len;
    while (end > start && (str.ptr[end - 1] == ' ' || str.ptr[end - 1] == '\t')) {
        end--;
    }
    return th_string_substr(str, start, end - start);
}

TH_PRIVATE(uint32_t)
th_string_hash(th_string str)
{
    return th_hash_bytes(str.ptr, str.len);
}
