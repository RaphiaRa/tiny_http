#include "th_dir.h"
#include "th_config.h"
#include "th_utility.h"
#include "th_path.h"

#if defined(TH_CONFIG_OS_POSIX)
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#elif defined(TH_CONFIG_OS_MOCK)
#include "th_mock_syscall.h"
#endif

TH_PRIVATE(void)
th_dir_init(th_dir* dir, th_allocator* allocator)
{
    dir->allocator = allocator ? allocator : th_default_allocator_get();
    dir->fd = -1;
    th_heap_string_init(&dir->path, dir->allocator);
}

TH_PRIVATE(th_err)
th_dir_open(th_dir* dir, th_string path)
{
    th_err err = TH_ERR_OK;
    if ((err = th_path_resolve(path, &dir->path)) != TH_ERR_OK)
        return err;
#if defined(TH_CONFIG_OS_POSIX)
    if (path.len > TH_CONFIG_MAX_PATH_LEN)
        return TH_ERR_INVALID_ARG;
    char path_buf[TH_CONFIG_MAX_PATH_LEN + 1] = {0};
    memcpy(path_buf, path.ptr, path.len);
    path_buf[path.len] = '\0';
    int fd = open(path_buf, O_RDONLY | O_DIRECTORY);
    if (fd < 0)
        return TH_ERR_SYSTEM(errno);
    dir->fd = fd;
    return TH_ERR_OK;
#elif defined(TH_CONFIG_OS_MOCK)
    (void)path;
    int fd = th_mock_open();
    if (fd < 0)
        return TH_ERR_SYSTEM(-fd);
    dir->fd = fd;
    return TH_ERR_OK;
#endif
}

TH_PRIVATE(th_string)
th_dir_get_path(th_dir* dir)
{
    return th_heap_string_view(&dir->path);
}

TH_PRIVATE(void)
th_dir_deinit(th_dir* dir)
{
    th_heap_string_deinit(&dir->path);
#if defined(TH_CONFIG_OS_POSIX)
    if (dir->fd >= 0) {
        int ret = close(dir->fd);
        (void)ret;
        TH_ASSERT(ret == 0 && "This should not happen");
    }
#elif defined(TH_CONFIG_OS_MOCK)
    if (dir->fd >= 0) {
        int ret = th_mock_close();
        (void)ret;
        TH_ASSERT(ret == 0 && "This should not happen");
    }
#endif
}
