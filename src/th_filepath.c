#include "th_filepath.h"

#include <string.h>

TH_PRIVATE(th_err)
th_filepath_init(th_filepath* path, th_str str)
{
    if (str.len == 0 || str.len > TH_CONFIG_MAX_PATH_LEN)
        return TH_ERR_INVALID_ARG;
    if (str.ptr[0] == '/' || str.ptr[str.len - 1] == '/')
        return TH_ERR_INVALID_ARG;
    if (th_str_find_first(str, 0, '\0') != th_str_npos)
        return TH_ERR_INVALID_ARG;
    size_t out = 0;
    size_t start = 0;
    while (start < str.len) {
        size_t sep = th_str_find_first(str, start, '/');
        size_t end = sep == th_str_npos ? str.len : sep;
        size_t len = end - start;
        if (len == 2 && str.ptr[start] == '.' && str.ptr[start + 1] == '.')
            return TH_ERR_INVALID_ARG;
        bool is_dot = len == 1 && str.ptr[start] == '.';
        if (len > 0 && !is_dot) {
            if (out > 0)
                path->buf[out++] = '/';
            memcpy(path->buf + out, str.ptr + start, len);
            out += len;
        }
        start = end + 1;
    }
    if (out == 0)
        return TH_ERR_INVALID_ARG;
    path->buf[out] = '\0';
    return TH_ERR_OK;
}
