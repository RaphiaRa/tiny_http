#include "th_file.h"
#include "th_allocator.h"
#include "th_config.h"
#include "th_fmt.h"
#include "th_log.h"
#include "th_string.h"
#include "th_system_error.h"

#include <stdio.h>

#if defined(TH_CONFIG_OS_POSIX)
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#undef TH_LOG_TAG
#define TH_LOG_TAG "file"

/* th_file_ops implementation begin */

#if defined(TH_CONFIG_OS_POSIX)
TH_LOCAL(th_err)
th_file_ops_os_openat(void* self, int dirfd, const char* path, int flags, int* fd)
{
    (void)self;
    int ret = openat(dirfd, path, flags, 0644);
    if (ret == -1)
        return TH_ERR_SYSTEM(errno);
    *fd = ret;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_file_ops_os_seek(void* self, int fd, int whence, size_t* pos)
{
    (void)self;
    off_t ret = lseek(fd, 0, whence);
    if (ret == -1)
        return TH_ERR_SYSTEM(errno);
    *pos = (size_t)ret;
    return TH_ERR_OK;
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

TH_LOCAL(th_err)
th_file_ops_os_stat(void* self, int fd, struct stat* out)
{
    (void)self;
    if (fstat(fd, out) == -1)
        return TH_ERR_SYSTEM(errno);
    return TH_ERR_OK;
}

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
        .seek = th_file_ops_os_seek,
        .read = th_file_ops_os_read,
        .write = th_file_ops_os_write,
        .stat = th_file_ops_os_stat,
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

TH_LOCAL(int)
th_open_opt_to_flags(th_open_opt opt)
{
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
    return flags;
}

TH_PRIVATE(th_err)
th_file_openat(th_file* stream, th_dir* dir, const th_filepath* path, th_open_opt opt)
{
    int fd = -1;
    th_err err = stream->ops->openat(stream->ops, dir->fd, th_filepath_cstr(path), th_open_opt_to_flags(opt), &fd);
    if (err != TH_ERR_OK)
        return err;
    size_t size = 0;
    size_t unused = 0;
    if ((err = stream->ops->seek(stream->ops, fd, SEEK_END, &size)) != TH_ERR_OK)
        goto cleanup;
    if ((err = stream->ops->seek(stream->ops, fd, SEEK_SET, &unused)) != TH_ERR_OK)
        goto cleanup;
    stream->fd = fd;
    stream->size = size;
    return TH_ERR_OK;
cleanup:
    stream->ops->close(stream->ops, fd);
    return err;
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

/**
 * We use DJB2 hash function, without multiplication,
 * as it's faster and good enough for our purposes.
 */
#define FSTAT_HASH_INIT 5381
#define FSTAT_HASH_NEXT(hash, val) ((hash << 5) + hash + val)

TH_PRIVATE(uint32_t)
th_file_stat_hash(th_file* stream)
{
    struct stat st = {0};
    th_err err = stream->ops->stat(stream->ops, stream->fd, &st);
    if (err != TH_ERR_OK) {
        TH_LOG_ERROR("stat failed: %s, can't calculate hash", th_strerror(err));
        TH_ASSERT(0 && "stat failed");
        return 0;
    }
#if defined(TH_CONFIG_OS_OSX)
    int64_t mtime_sec = st.st_mtimespec.tv_sec;
    int64_t mtime_nsec = st.st_mtimespec.tv_nsec;
#else
    int64_t mtime_sec = st.st_mtime;
    int64_t mtime_nsec = 0;
#endif
    uint32_t hash = FSTAT_HASH_INIT;
    hash = FSTAT_HASH_NEXT(hash, (uint32_t)mtime_sec);
    hash = FSTAT_HASH_NEXT(hash, (uint32_t)mtime_nsec);
    hash = FSTAT_HASH_NEXT(hash, (uint32_t)st.st_size);
    hash = FSTAT_HASH_NEXT(hash, st.st_mode);
    hash = FSTAT_HASH_NEXT(hash, (uint32_t)st.st_ino);
    hash = FSTAT_HASH_NEXT(hash, st.st_uid);
    hash = FSTAT_HASH_NEXT(hash, st.st_gid);
    hash = FSTAT_HASH_NEXT(hash, (uint32_t)(st.st_nlink != 0));
    return hash;
}
#undef FSTAT_HASH_INIT
#undef FSTAT_HASH_NEXT

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
