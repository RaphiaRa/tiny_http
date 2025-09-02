
#include "th_io_op_posix.h"

#if defined(TH_CONFIG_OS_POSIX)

#include "th_align.h"
#include "th_io_task.h"
#include "th_log.h"
#include "th_utility.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>

#if defined(TH_CONFIG_OS_BSD)
#define CAST_MSG_IOVLEN(len) ((int)(len))
#else
#define CAST_MSG_IOVLEN(len) ((size_t)(len))
#endif

TH_PRIVATE(th_err)
th_io_op_posix_read(void* self, size_t* result)
{
    th_err err = TH_ERR_OK;
    th_io_task* iot = self;
    ssize_t ret = read(iot->fd, iot->addr, iot->len);
    if (ret < 0)
        err = TH_ERR_SYSTEM(errno);
    else if (ret == 0)
        err = TH_ERR_EOF;
    (*result) = (size_t)ret;
    return err;
}

TH_PRIVATE(th_err)
th_io_op_posix_readv(void* self, size_t* result)
{
    th_err err = TH_ERR_OK;
    th_io_task* iot = self;
    th_iov* iov = iot->addr;
    ssize_t ret = readv(iot->fd, (struct iovec*)iov, (int)iot->len);
    if (ret < 0)
        err = TH_ERR_SYSTEM(errno);
    else if (ret == 0)
        err = TH_ERR_EOF;
    (*result) = (size_t)ret;
    return err;
}

TH_PRIVATE(th_err)
th_io_op_posix_write(void* self, size_t* result)
{
    th_err err = TH_ERR_OK;
    th_io_task* iot = self;
    ssize_t ret = write(iot->fd, iot->addr, iot->len);
    if (ret < 0)
        err = TH_ERR_SYSTEM(errno);
    (*result) = (size_t)ret;
    return err;
}

TH_PRIVATE(th_err)
th_io_op_posix_writev(void* self, size_t* result)
{
    th_err err = TH_ERR_OK;
    th_io_task* iot = self;
    th_iov* iov = iot->addr;
    ssize_t ret = writev(iot->fd, (struct iovec*)iov, (int)iot->len);
    if (ret < 0)
        err = TH_ERR_SYSTEM(errno);
    (*result) = (size_t)ret;
    return err;
}

TH_PRIVATE(th_err)
th_io_op_posix_send(void* self, size_t* result)
{
    th_err err = TH_ERR_OK;
    th_io_task* iot = self;
    int flags = 0;
#if defined(MSG_NOSIGNAL)
    flags |= MSG_NOSIGNAL;
#endif
    ssize_t ret = send(iot->fd, iot->addr, iot->len, flags);
    if (ret < 0)
        err = TH_ERR_SYSTEM(errno);
    (*result) = (size_t)ret;
    return err;
}

TH_PRIVATE(th_err)
th_io_op_posix_sendv(void* self, size_t* result)
{
    th_err err = TH_ERR_OK;
    th_io_task* iot = self;
    int flags = 0;
#if defined(MSG_NOSIGNAL)
    flags |= MSG_NOSIGNAL;
#endif
    struct msghdr msg = {0};
    msg.msg_iov = iot->addr;
    msg.msg_iovlen = CAST_MSG_IOVLEN(iot->len);
    ssize_t ret = sendmsg(iot->fd, &msg, flags);
    if (ret < 0)
        err = TH_ERR_SYSTEM(errno);
    (*result) = (size_t)ret;
    return err;
}

TH_PRIVATE(th_err)
th_io_op_posix_accept(void* self, size_t* result)
{
    th_err err = TH_ERR_OK;
    th_io_task* iot = self;
    int ret = accept(iot->fd, iot->addr, iot->addr2);
    if (ret < 0)
        err = TH_ERR_SYSTEM(errno);
    (*result) = (size_t)ret;
    return err;
}

TH_PRIVATE(th_err)
th_io_op_posix_sendfile_mmap(void* self, size_t* result)
{
    th_err err = TH_ERR_OK;
    th_io_task* iot = self;
    th_file* file = iot->addr2;
    th_fileview view;
    if ((err = th_file_get_view(file, &view, iot->offset, iot->len2)) != TH_ERR_OK)
        return err;
    struct iovec vec[64];
    size_t veclen = 0;
    if (iot->len > 0) {
        th_iov* iov = iot->addr;
        for (size_t i = 0; i < iot->len; i++) {
            vec[i].iov_base = iov[i].base;
            vec[i].iov_len = iov[i].len;
            veclen++;
        }
    }
    vec[veclen].iov_base = view.ptr;
    vec[veclen].iov_len = view.len;
    veclen++;
    int flags = 0;
#if defined(MSG_NOSIGNAL)
    flags |= MSG_NOSIGNAL;
#endif
    struct msghdr msg = {0};
    msg.msg_iov = vec;
    msg.msg_iovlen = CAST_MSG_IOVLEN(veclen);
    ssize_t ret = sendmsg(iot->fd, &msg, flags);
    if (ret < 0)
        err = TH_ERR_SYSTEM(errno);
    *result = (size_t)ret;
    return err;
}

#define TH_IO_OP_POSIX_SENDFILE_BUFFERED_MAX 8 * 1024
TH_PRIVATE(th_err)
th_io_op_posix_sendfile_buffered(void* self, size_t* result)
{
    uint8_t buffer[TH_IO_OP_POSIX_SENDFILE_BUFFERED_MAX];
    th_io_task* iot = self;
    struct iovec vec[64];
    size_t veclen = 0;
    if (iot->len > 0) {
        th_iov* iov = iot->addr;
        for (size_t i = 0; i < iot->len; i++) {
            vec[i].iov_base = iov[i].base;
            vec[i].iov_len = iov[i].len;
            veclen++;
        }
    }
    size_t toread = TH_MIN(sizeof(buffer), iot->len2);
    ssize_t readlen = pread(((th_file*)iot->addr2)->fd, buffer, toread, (off_t)iot->offset);
    if (readlen < 0)
        return TH_ERR_SYSTEM(errno);
    int flags = 0;
#if defined(MSG_NOSIGNAL)
    flags |= MSG_NOSIGNAL;
#endif
    vec[veclen].iov_base = buffer;
    vec[veclen].iov_len = (size_t)readlen;
    veclen++;
    struct msghdr msg = {0};
    msg.msg_iov = vec;
    msg.msg_iovlen = CAST_MSG_IOVLEN(veclen);
    ssize_t writelen = sendmsg(iot->fd, &msg, flags);
    if (writelen < 0)
        return TH_ERR_SYSTEM(errno);
    *result = (size_t)writelen;
    return TH_ERR_OK;
}

#endif
