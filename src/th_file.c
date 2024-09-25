#include "th_file.h"
#include "th_align.h"
#include "th_allocator.h"
#include "th_config.h"
#include "th_fmt.h"
#include "th_heap_string.h"
#include "th_path.h"

#if defined(TH_CONFIG_OS_POSIX)
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#elif defined(TH_CONFIG_OS_MOCK)
#include "th_mock_syscall.h"
#endif

/* th_file_view implmentation begin */

#if defined(TH_CONFIG_OS_POSIX)
TH_LOCAL(th_err)
th_file_mmap_mmap_posix(th_file_mmap* view, th_file* file, size_t offset, size_t len)
{
    size_t page_size = (size_t)sysconf(_SC_PAGESIZE);
    size_t moffset = TH_ALIGNDOWN(offset, page_size);
    void* addr = mmap(NULL, len, PROT_READ, MAP_PRIVATE, file->fd, moffset);
    if (addr == MAP_FAILED) {
        return TH_ERR_SYSTEM(errno);
    }
    view->addr = addr;
    view->offset = moffset;
    view->len = len;
    return TH_ERR_OK;
}

TH_LOCAL(void)
th_file_mmap_munmap_posix(th_file_mmap* view)
{
    munmap(view->addr, view->len);
    view->addr = 0;
    view->len = 0;
    view->offset = 0;
}
#endif

TH_LOCAL(th_err)
th_file_mmap_mmap(th_file_mmap* view, th_file* file, size_t offset, size_t len)
{
#if defined(TH_CONFIG_OS_POSIX)
    return th_file_mmap_mmap_posix(view, file, offset, len);
#else
    (void)view;
    (void)file;
    (void)offset;
    (void)len;
    return TH_ERR_NOSUPPORT;
#endif
}

TH_LOCAL(void)
th_file_mmap_munmap(th_file_mmap* view)
{
#if defined(TH_CONFIG_OS_POSIX)
    th_file_mmap_munmap_posix(view);
#else
    (void)view;
#endif
}

TH_LOCAL(void)
th_file_mmap_init(th_file_mmap* view)
{
    view->addr = 0;
    view->offset = 0;
    view->len = 0;
}

TH_LOCAL(th_err)
th_file_mmap_map(th_file_mmap* view, th_file* file, size_t offset, size_t len)
{
    if (view->addr)
        th_file_mmap_munmap(view);
    len = TH_MIN(len, file->size - offset);
    return th_file_mmap_mmap(view, file, offset, len);
}

TH_LOCAL(void)
th_file_mmap_deinit(th_file_mmap* view)
{
    if (view->addr)
        th_file_mmap_munmap(view);
}

/* th_file_mmap_map implementation end */
/* th_file implementation begin */

TH_LOCAL(th_err)
th_file_validate_path(th_dir* dir, th_string path, th_allocator* allocator)
{
    if (path.len > TH_CONFIG_MAX_PATH_LEN)
        return TH_ERR_INVALID_ARG;
    th_heap_string realpath = {0};
    th_heap_string_init(&realpath, allocator);
    th_err err = TH_ERR_OK;
    if ((err = th_path_resolve_against(path, dir, &realpath)) != TH_ERR_OK)
        goto cleanup;
    if (!th_path_is_within(th_heap_string_view(&realpath), dir)) {
        err = TH_ERR_HTTP(TH_CODE_FORBIDDEN);
        goto cleanup;
    }
    if (th_path_is_hidden(th_heap_string_view(&realpath))) {
        err = TH_ERR_HTTP(TH_CODE_FORBIDDEN);
        goto cleanup;
    }
cleanup:
    th_heap_string_deinit(&realpath);
    return err;
}

TH_PRIVATE(void)
th_file_init(th_file* stream)
{
    stream->fd = -1;
    th_file_mmap_init(&stream->view);
}

TH_PRIVATE(th_err)
th_file_openat(th_file* stream, th_dir* dir, th_string path, th_open_opt opt)
{
    th_err err = TH_ERR_OK;
    if ((err = th_file_validate_path(dir, path, dir->allocator)) != TH_ERR_OK)
        return err;
#if defined(TH_CONFIG_OS_POSIX)
    char path_buf[TH_CONFIG_MAX_PATH_LEN + 1] = {0};
    memcpy(path_buf, path.ptr, path.len);
    path_buf[path.len] = '\0';
    int flags = O_NOFOLLOW;
    if (opt.read && opt.write)
        flags = O_RDWR;
    else if (opt.read)
        flags = O_RDONLY;
    else if (opt.write)
        flags = O_WRONLY;
    int fd = openat(dir->fd, path_buf, flags);
    if (fd == -1)
        return TH_ERR_SYSTEM(errno);
    off_t pos = lseek(fd, 0, SEEK_END);
    if (pos == -1)
        goto cleanup_socket;
    if (lseek(fd, 0, SEEK_SET) == -1)
        goto cleanup_socket;
    stream->fd = fd;
    stream->size = (size_t)pos;
    return TH_ERR_OK;
cleanup_socket:
    close(fd);
    return TH_ERR_SYSTEM(errno);
#elif defined(TH_CONFIG_OS_MOCK)
    (void)dir;
    (void)opt;
    (void)path;
    int fd = th_mock_open();
    if (fd < 0)
        return TH_ERR_SYSTEM(-fd);
    stream->fd = fd;
    return TH_ERR_OK;
#endif
}

TH_PRIVATE(th_err)
th_file_read(th_file* stream, void* addr, size_t len, size_t offset, size_t* read)
{
#if defined(TH_CONFIG_OS_POSIX)
    off_t ret = pread(stream->fd, addr, len, offset);
    if (ret == -1) {
        *read = 0;
        return TH_ERR_SYSTEM(errno);
    }
    *read = (size_t)ret;
    return TH_ERR_OK;
#elif defined(TH_CONFIG_OS_MOCK)
    (void)stream;
    (void)offset;
    int ret = th_mock_read(addr, len);
    if (ret < 0)
        return TH_ERR_SYSTEM(-ret);
    *read = (size_t)ret;
    return TH_ERR_OK;
#endif
}

TH_PRIVATE(th_err)
th_file_get_view(th_file* stream, th_fileview* view, size_t offset, size_t len)
{
    th_err err = TH_ERR_OK;
    if (stream->view.addr == NULL
        || stream->view.offset > offset
        || stream->view.offset + stream->view.len < offset + 8 * 1024) {
        if ((err = th_file_mmap_map(&stream->view, stream, offset, len)) != TH_ERR_OK)
            return err;
    }
    view->ptr = (uint8_t*)stream->view.addr + (offset - stream->view.offset);
    view->len = stream->view.len - (offset - stream->view.offset);
    return TH_ERR_OK;
}

TH_PRIVATE(void)
th_file_close(th_file* stream)
{
    th_file_mmap_deinit(&stream->view);
#if defined(TH_CONFIG_OS_POSIX)
    if (stream->fd != -1)
        close(stream->fd);
#elif defined(TH_CONFIG_OS_MOCK)
    if (stream->fd != -1)
        th_mock_close();
#endif
    stream->fd = -1;
}

TH_PRIVATE(void)
th_file_deinit(th_file* stream)
{
    th_file_close(stream);
}
