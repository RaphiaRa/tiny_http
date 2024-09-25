#include "th_path.h"

#include "th_config.h"
#include "th_fmt.h"
#include "th_string.h"

#include <stdlib.h>

#if defined(TH_CONFIG_OS_POSIX)
#include <errno.h>
#include <limits.h>

TH_LOCAL(th_err)
th_path_resolve_posix(th_string path, th_heap_string* out)
{
    char in[TH_CONFIG_MAX_PATH_LEN + 1] = {0};
    size_t pos = 0;
    pos += th_fmt_strn_append(in, pos, sizeof(in) - pos, path.ptr, path.len);
    in[pos] = '\0';
    th_heap_string_resize(out, TH_CONFIG_MAX_PATH_LEN, 0);
    char* out_ptr = th_heap_string_data(out);
    char* ret = realpath(in, out_ptr);
    if (ret == NULL)
        return TH_ERR_SYSTEM(errno);
    th_heap_string_resize(out, strlen(out_ptr), 0);
    return TH_ERR_OK;
}
/*
TH_LOCAL(th_err)
th_path_resolve_against_posix(th_dir* dir, th_string path, th_heap_string* out)
{
    char in[TH_CONFIG_MAX_PATH_LEN + 1] = {0};
    th_string root = th_dir_get_path(dir);
    size_t pos = 0;
    pos += th_fmt_strn_append(in, pos, sizeof(in) - pos, root.ptr, root.len);
    pos += th_fmt_str_append(in, pos, sizeof(in) - pos, "/");
    pos += th_fmt_strn_append(in, pos, sizeof(in) - pos, path.ptr, path.len);
    th_heap_string_resize(out, TH_CONFIG_MAX_PATH_LEN, 0);
    char* out_ptr = th_heap_string_data(out);
    char* ret = realpath(in, out_ptr);
    if (ret == NULL)
        return TH_ERR_SYSTEM(errno);
    th_heap_string_resize(out, strlen(out_ptr), 0);

    return TH_ERR_OK;
}
*/
#elif defined(TH_CONFIG_OS_MOCK)
TH_LOCAL(th_err)
th_path_resolve_mock(th_string path, th_heap_string* out)
{
    (void)path;
    th_heap_string_clear(out);
    th_heap_string_set(out, path);
    return TH_ERR_OK;
}
#endif

TH_PRIVATE(th_err)
th_path_resolve(th_string path, th_heap_string* out)
{
#if defined(TH_CONFIG_OS_POSIX)
    return th_path_resolve_posix(path, out);
#elif defined(TH_CONFIG_OS_MOCK)
    return th_path_resolve_mock(path, out);
#else
    (void)path;
    (void)out;
    TH_ASSERT(0 && "Not implemented");
    return TH_ERR_NOSUPPORT;
#endif
}

TH_PRIVATE(th_err)
th_path_resolve_against(th_string path, th_dir* dir, th_heap_string* out)
{
    char in[TH_CONFIG_MAX_PATH_LEN + 1] = {0};
    th_string root = th_dir_get_path(dir);
    size_t pos = 0;
    pos += th_fmt_strn_append(in, pos, sizeof(in) - pos, root.ptr, root.len);
    pos += th_fmt_str_append(in, pos, sizeof(in) - pos, "/");
    pos += th_fmt_strn_append(in, pos, sizeof(in) - pos, path.ptr, path.len);
    return th_path_resolve(th_string_make(in, pos), out);
}

TH_PRIVATE(bool)
th_path_is_within(th_string realpath, th_dir* dir)
{
    th_string root = th_dir_get_path(dir);
    if (realpath.len < root.len)
        return false;
    return th_string_eq(th_string_make(realpath.ptr, root.len), root);
}

TH_PRIVATE(bool)
th_path_is_hidden(th_string path)
{
    size_t pos = 0;
    while ((pos = th_string_find_first(path, pos, '/')) != th_string_npos) {
        if (path.ptr[++pos] == '.')
            return true;
    }
    return false;
}
