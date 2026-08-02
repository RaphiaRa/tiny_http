#include "th_dir.h"
#include "th_config.h"
#include "th_path.h"
#include "th_utility.h"

#if defined(TH_CONFIG_OS_POSIX)
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

TH_LOCAL(th_err)
th_dir_ops_os_open(void* self, const char* path, int* fd)
{
    (void)self;
    int ret = open(path, O_RDONLY | O_DIRECTORY);
    if (ret < 0)
        return TH_ERR_SYSTEM(errno);
    *fd = ret;
    return TH_ERR_OK;
}

TH_LOCAL(void)
th_dir_ops_os_close(void* self, int fd)
{
    (void)self;
    int ret = close(fd);
    (void)ret;
    TH_ASSERT(ret == 0 && "This should not happen");
}

TH_PRIVATE(th_dir_ops*)
th_dir_ops_os(void)
{
    static th_dir_ops ops = {
        .open = th_dir_ops_os_open,
        .close = th_dir_ops_os_close,
    };
    return &ops;
}
#endif

TH_PRIVATE(void)
th_dir_init(th_dir* dir, th_dir_ops* ops, th_allocator* allocator)
{
    dir->allocator = allocator ? allocator : th_default_allocator_get();
    dir->ops = ops;
    dir->fd = -1;
    th_string_init(&dir->path, dir->allocator);
}

TH_PRIVATE(th_err)
th_dir_open(th_dir* dir, th_str path)
{
    th_err err = TH_ERR_OK;
    if ((err = th_path_resolve(path, &dir->path)) != TH_ERR_OK)
        return err;
    if (path.len > TH_CONFIG_MAX_PATH_LEN)
        return TH_ERR_INVALID_ARG;
    char path_buf[TH_CONFIG_MAX_PATH_LEN + 1] = {0};
    memcpy(path_buf, path.ptr, path.len);
    path_buf[path.len] = '\0';
    int fd = -1;
    if ((err = dir->ops->open(dir->ops, path_buf, &fd)) != TH_ERR_OK)
        return err;
    dir->fd = fd;
    return TH_ERR_OK;
}

TH_PRIVATE(th_str)
th_dir_get_path(th_dir* dir)
{
    return th_string_view(&dir->path);
}

TH_PRIVATE(void)
th_dir_deinit(th_dir* dir)
{
    th_string_deinit(&dir->path);
    if (dir->fd >= 0)
        dir->ops->close(dir->ops, dir->fd);
}
