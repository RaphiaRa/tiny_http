#ifndef TH_FMT_H
#define TH_FMT_H

#include <th.h>

#include <stddef.h>
#include <stdint.h>
#include <time.h>

#include "th_config.h"

TH_PRIVATE(const char*)
th_fmt_uint_to_str(char* buf, size_t len, unsigned int val);

TH_PRIVATE(const char*)
th_fmt_uint_to_str_ex(char* buf, size_t len, unsigned int val, size_t* out_len);

/** th_fmt_str_append
 * @brief Append a string to a buffer.
 * @param buf The buffer to append to.
 * @param pos The current position in the buffer (where to append).
 * @param len The length of the buffer.
 * @param str The string to append.
 * @return The number of characters appended.
 */
TH_PRIVATE(size_t)
th_fmt_str_append(char* buf, size_t pos, size_t len, const char* str);

TH_PRIVATE(size_t)
th_fmt_strn_append(char* buf, size_t pos, size_t len, const char* str, size_t n);

TH_PRIVATE(size_t)
th_fmt_strtime(char* buf, size_t len, th_date date);
#endif
