#include "th_file.h"
#include "th_allocator.h"
#include "th_config.h"
#include "th_fmt.h"
#include "th_log.h"
#include "th_string.h"
#include "th_system_error.h"

#if defined(TH_CONFIG_OS_POSIX)
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#undef TH_LOG_TAG
#define TH_LOG_TAG "file"

/* th_file_ops implementation begin */

#if defined(TH_CONFIG_OS_POSIX)
TH_LOCAL(th_err)
th_file_ops_os_openat(void* self, th_dir* dir, th_str path, th_open_opt opt, int* fd, size_t* size)
{
    (void)self;
    if (path.len > TH_CONFIG_MAX_PATH_LEN)
        return TH_ERR_INVALID_ARG;
    char path_buf[TH_CONFIG_MAX_PATH_LEN + 1] = {0};
    memcpy(path_buf, path.ptr, path.len);
    path_buf[path.len] = '\0';
    int flags = O_NOFOLLOW;
    if (opt.read && opt.write)
        flags |= O_RDWR;
    else if (opt.read)
        flags |= O_RDONLY;
    else if (opt.write)
        flags |= O_WRONLY;
    if (opt.create)
        flags |= O_CREAT;
    if (opt.truncate)
        flags |= O_TRUNC;
    int ret = openat(dir->fd, path_buf, flags, 0644);
    if (ret == -1)
        return TH_ERR_SYSTEM(errno);
    off_t pos = lseek(ret, 0, SEEK_END);
    if (pos == -1)
        goto cleanup;
    if (lseek(ret, 0, SEEK_SET) == -1)
        goto cleanup;
    *fd = ret;
    *size = (size_t)pos;
    return TH_ERR_OK;
cleanup:
    close(ret);
    return TH_ERR_SYSTEM(errno);
}

TH_LOCAL(th_err)
th_file_ops_os_read(void* self, int fd, void* addr, size_t len, size_t offset, size_t* read)
{
    (void)self;
    off_t ret = pread(fd, addr, len, (off_t)offset);
    if (ret == -1) {
        *read = 0;
        return TH_ERR_SYSTEM(errno);
    }
    *read = (size_t)ret;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_file_ops_os_write(void* self, int fd, const void* addr, size_t len, size_t offset, size_t* written)
{
    (void)self;
    off_t ret = pwrite(fd, addr, len, (off_t)offset);
    if (ret == -1) {
        *written = 0;
        return TH_ERR_SYSTEM(errno);
    }
    *written = (size_t)ret;
    return TH_ERR_OK;
}

/**
 * We use DJB2 hash function, without multiplication,
 * as it's faster and good enough for our purposes.
 */
#define FSTAT_HASH_INIT 5381
#define FSTAT_HASH_NEXT(hash, val) ((hash << 5) + hash + val)

TH_LOCAL(uint32_t)
th_file_ops_os_stat_hash(void* self, int fd)
{
    (void)self;
    struct stat st = {0};
    if (fstat(fd, &st) == -1) {
        TH_LOG_ERROR("fstat failed: %s, can't calculate hash", strerror(errno));
        TH_ASSERT(0 && "fstat failed");
        return 0;
    }

    uint32_t hash = FSTAT_HASH_INIT;
#if defined(TH_CONFIG_OS_OSX)
    hash = FSTAT_HASH_NEXT(hash, (uint32_t)st.st_mtimespec.tv_sec);
    hash = FSTAT_HASH_NEXT(hash, (uint32_t)st.st_mtimespec.tv_nsec);
#else
    hash = FSTAT_HASH_NEXT(hash, (uint32_t)st.st_mtime);
#endif
    hash = FSTAT_HASH_NEXT(hash, (uint32_t)st.st_size);
    hash = FSTAT_HASH_NEXT(hash, (uint32_t)st.st_mode);
    hash = FSTAT_HASH_NEXT(hash, (uint32_t)st.st_ino);
    hash = FSTAT_HASH_NEXT(hash, (uint32_t)st.st_uid);
    hash = FSTAT_HASH_NEXT(hash, (uint32_t)st.st_gid);
    hash = FSTAT_HASH_NEXT(hash, (uint32_t)(st.st_nlink != 0));
    return hash;
}
#undef FSTAT_HASH_INIT
#undef FSTAT_HASH_NEXT

TH_LOCAL(void)
th_file_ops_os_close(void* self, int fd)
{
    (void)self;
    close(fd);
}

TH_PRIVATE(th_file_ops*)
th_file_ops_os(void)
{
    static th_file_ops ops = {
        .openat = th_file_ops_os_openat,
        .read = th_file_ops_os_read,
        .write = th_file_ops_os_write,
        .stat_hash = th_file_ops_os_stat_hash,
        .close = th_file_ops_os_close,
    };
    return &ops;
}
#endif

/* th_file_ops implementation end */
/* th_file implementation begin */

TH_PRIVATE(void)
th_file_init(th_file* stream, th_file_ops* ops)
{
    stream->ops = ops;
    stream->fd = -1;
}

TH_PRIVATE(th_err)
th_file_openat(th_file* stream, th_dir* dir, th_str path, th_open_opt opt)
{
    int fd = -1;
    size_t size = 0;
    th_err err = stream->ops->openat(stream->ops, dir, path, opt, &fd, &size);
    if (err != TH_ERR_OK)
        return err;
    stream->fd = fd;
    stream->size = size;
    return TH_ERR_OK;
}

TH_PRIVATE(th_err)
th_file_read(th_file* stream, void* addr, size_t len, size_t offset, size_t* read)
{
    return stream->ops->read(stream->ops, stream->fd, addr, len, offset, read);
}

TH_PRIVATE(th_err)
th_file_write(th_file* stream, const void* addr, size_t len, size_t offset, size_t* written)
{
    return stream->ops->write(stream->ops, stream->fd, addr, len, offset, written);
}

TH_PRIVATE(uint32_t)
th_file_stat_hash(th_file* stream)
{
    return stream->ops->stat_hash(stream->ops, stream->fd);
}

TH_PRIVATE(void)
th_file_close(th_file* stream)
{
    if (stream->fd != -1)
        stream->ops->close(stream->ops, stream->fd);
    stream->fd = -1;
}

TH_PRIVATE(void)
th_file_deinit(th_file* stream)
{
    th_file_close(stream);
}
