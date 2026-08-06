#ifndef TH_FILEPATH_H
#define TH_FILEPATH_H

#include <th.h>

#include "th_config.h"
#include "th_str.h"

/** th_filepath
 * @brief A validated, NUL-terminated relative path, ready to pass to a
 * syscall. th_filepath_init rejects absolute paths and "." / ".."
 * components - openat(dir->fd, ...) doesn't confine resolution to dir, a
 * ".." component walks back out of it like normal path resolution - so
 * any path built from untrusted input (e.g. a client-supplied filename)
 * must go through this first.
 */
typedef struct th_filepath {
    char buf[TH_CONFIG_MAX_PATH_LEN + 1];
} th_filepath;

/** th_filepath_init
 * @brief Fills path with str NUL-terminated.
 * @return TH_ERR_INVALID_ARG if str is absolute, too long, empty, or has
 * a "." / ".." component.
 */
TH_PRIVATE(th_err)
th_filepath_init(th_filepath* path, th_str str);

TH_INLINE(const char*)
th_filepath_cstr(const th_filepath* path)
{
    return path->buf;
}

#endif
