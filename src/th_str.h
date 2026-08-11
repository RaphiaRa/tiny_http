#ifndef TH_STR_H
#define TH_STR_H

#include <stdint.h>
#include <string.h>

#include <th.h>

#include "th_config.h"

extern size_t th_str_npos;

typedef struct th_str {
    const char* ptr;
    size_t len;
} th_str;

/** th_str_make
 * @brief Helper function to create a th_str from a pointer and a length.
 */
TH_INLINE(th_str)
th_str_make(const char* ptr, size_t len)
{
    return (th_str){ptr, len};
}

/** th_str_make_empty
 * @brief Helper function to create an empty th_str.
 */
TH_INLINE(th_str)
th_str_make_empty(void)
{
    return (th_str){"", 0};
}

/** th_str_from_cstr
 * @brief Helper function to create a th_str from a null-terminated string.
 */
TH_INLINE(th_str)
th_str_from_cstr(const char* str)
{
    return th_str_make(str, strlen(str));
}

/** th_str_eq
 * @brief Helper function to compare two th_strs.
 * @return 1 if the strings are equal, 0 otherwise.
 */
TH_PRIVATE(bool)
th_str_eq(th_str a, th_str b);

/** th_str_ieq
 * @brief Case-insensitive version of th_str_eq.
 * @return 1 if the strings are equal ignoring case, 0 otherwise.
 */
TH_PRIVATE(bool)
th_str_ieq(th_str a, th_str b);

/** th_str_empty
 * @brief Helper function to check if a th_str is empty.
 * @return true if the string is empty, false otherwise.
 */
TH_INLINE(bool)
th_str_empty(th_str str)
{
    return str.len == 0;
}

/** TH_STR_INIT
 * @brief Helper macro to initialize a th_str from string literal.
 */
#define TH_STR_INIT(str) {"" str, sizeof(str) - 1}

/** TH_STR
 * @brief Helper macro to create a th_str compound literal from a string literal.
 */
#define TH_STR(str) ((th_str){"" str, sizeof(str) - 1})

/** TH_STR_EQ
 * @brief Helper macro to compare a th_str with a string literal.
 */
#define TH_STR_EQ(str, cmp) (th_str_eq(str, TH_STR(cmp)))

TH_PRIVATE(bool)
th_str_is_uint(th_str str);

TH_PRIVATE(th_err)
th_str_to_uint(th_str str, unsigned int* out);

TH_PRIVATE(size_t)
th_str_find_first(th_str str, size_t start, char c);

TH_PRIVATE(size_t)
th_str_find_first_not(th_str str, size_t start, char c);

TH_PRIVATE(size_t)
th_str_find_first_of(th_str str, size_t start, const char* chars);

TH_PRIVATE(size_t)
th_str_find_last(th_str str, size_t start, char c);

/** th_str_substr
 * @brief Returns a substring of a string.
 * If len == th_str_npos, the substring will go to the end of the string.
 * If start > len, an empty string is returned (ptr = str.ptr + str.len, len = 0).
 */
TH_PRIVATE(th_str)
th_str_substr(th_str str, size_t start, size_t len);

/** th_str_trim
 * @brief Removes leading and trailing whitespace from a string.
 * This doesn't modify the original string, just returns a new view of it.
 * @param str The string to trim.
 * @return A new string view with leading and trailing whitespace removed.
 */
TH_PRIVATE(th_str)
th_str_trim(th_str str);

TH_PRIVATE(size_t)
th_str_hash(th_str str);

#endif
