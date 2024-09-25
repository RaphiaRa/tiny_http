#ifndef TH_STRING_H
#define TH_STRING_H

#include <stdint.h>
#include <string.h>

#include <th.h>

#include "th_config.h"

extern size_t th_string_npos;

typedef struct th_string {
    const char* ptr;
    size_t len;
} th_string;

/** th_string_make
 * @brief Helper function to create a th_string from a pointer and a length.
 */
TH_INLINE(th_string)
th_string_make(const char* ptr, size_t len)
{
    return (th_string){ptr, len};
}

/** th_string_from_cstr
 * @brief Helper function to create a th_string from a null-terminated string.
 */
TH_INLINE(th_string)
th_string_from_cstr(const char* str)
{
    return th_string_make(str, strlen(str));
}

/** th_string_eq
 * @brief Helper function to compare two th_strings.
 * @return 1 if the strings are equal, 0 otherwise.
 */
TH_PRIVATE(bool)
th_string_eq(th_string a, th_string b);

/** th_string_empty
 * @brief Helper function to check if a th_string is empty.
 * @return true if the string is empty, false otherwise.
 */
TH_INLINE(bool)
th_string_empty(th_string str)
{
    return str.len == 0;
}

/** TH_STRING_INIT
 * @brief Helper macro to initialize a th_string from string literal.
 */
#define TH_STRING_INIT(str) {"" str, sizeof(str) - 1}

/** TH_STRING
 * @brief Helper macro to create a th_string compound literal from a string literal.
 */
#define TH_STRING(str) ((th_string){"" str, sizeof(str) - 1})

/** TH_STRING_EQ
 * @brief Helper macro to compare a th_string with a string literal.
 */
#define TH_STRING_EQ(str, cmp) (th_string_eq(str, TH_STRING(cmp)))

TH_PRIVATE(bool)
th_string_is_uint(th_string str);

TH_PRIVATE(th_err)
th_string_to_uint(th_string str, unsigned int* out);

TH_PRIVATE(size_t)
th_string_find_first(th_string str, size_t start, char c);

TH_PRIVATE(size_t)
th_string_find_first_not(th_string str, size_t start, char c);

TH_PRIVATE(size_t)
th_string_find_last_not(th_string str, size_t start, char c);

TH_PRIVATE(size_t)
th_string_find_first_of(th_string str, size_t start, const char* chars);

/*
TH_PRIVATE(size_t)
th_string_find_last(th_string str, size_t start, char c);
*/

TH_PRIVATE(th_string)
th_string_substr(th_string str, size_t start, size_t len);

/** th_string_trim
 * @brief Removes leading and trailing whitespace from a string.
 * This doesn't modify the original string, just returns a new view of it.
 * @param str The string to trim.
 * @return A new string view with leading and trailing whitespace removed.
 */
TH_PRIVATE(th_string)
th_string_trim(th_string str);

TH_PRIVATE(uint32_t)
th_string_hash(th_string str);

typedef struct th_mut_string {
    char* ptr;
    size_t len;
} th_mut_string;

TH_PRIVATE(void)
th_mut_string_tolower(th_mut_string str);

#endif
