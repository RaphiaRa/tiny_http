#include "th_socket.h"
#include "th_system_error.h"
#include "th_utility.h"

#if defined(TH_CONFIG_OS_POSIX)
#include <errno.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>
#elif defined(TH_CONFIG_OS_WIN)
#include <winsock2.h>
#endif

#if defined(TH_CONFIG_OS_OSX)
#include <limits.h>
#endif

#if defined(TH_CONFIG_OS_POSIX)

TH_LOCAL(th_err)
th_socket_ops_os_send(void* self, int fd, const void* addr, size_t len, size_t* result)
{
    (void)self;
    int flags = 0;
#if defined(MSG_NOSIGNAL)
    flags |= MSG_NOSIGNAL;
#endif
    ssize_t ret = send(fd, addr, len, flags);
    if (ret < 0)
        return TH_ERR_SYSTEM(errno);
    *result = (size_t)ret;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_socket_ops_os_sendvec(void* self, int fd, const th_iov* iov, size_t iovcnt, size_t* result)
{
    (void)self;
    int flags = 0;
#if defined(MSG_NOSIGNAL)
    flags |= MSG_NOSIGNAL;
#endif
    struct msghdr msg = {0};
    msg.msg_iov = (struct iovec*)iov;
#if defined(TH_CONFIG_OS_OSX)
    TH_ASSERT(iovcnt <= INT_MAX);
    msg.msg_iovlen = (int)iovcnt;
#else
    msg.msg_iovlen = iovcnt;
#endif
    ssize_t ret = sendmsg(fd, &msg, flags);
    if (ret < 0)
        return TH_ERR_SYSTEM(errno);
    *result = (size_t)ret;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_socket_ops_os_recv(void* self, int fd, void* addr, size_t len, size_t* result)
{
    (void)self;
    ssize_t ret = recv(fd, addr, len, 0);
    if (ret < 0)
        return TH_ERR_SYSTEM(errno);
    if (ret == 0)
        return TH_ERR_EOF;
    *result = (size_t)ret;
    return TH_ERR_OK;
}

/* Builds header iov + one trailing iov (extra) into vec, capped at
 * TH_SOCKET_SENDFILE_MAX_IOV entries; returns the combined iovec count. */
#define TH_SOCKET_SENDFILE_MAX_IOV 64

TH_LOCAL(size_t)
th_socket_build_sendfile_iov(struct iovec* vec, const th_iov* iov, size_t iovcnt, void* extra_base, size_t extra_len)
{
    size_t veclen = 0;
    for (size_t i = 0; i < iovcnt && veclen < TH_SOCKET_SENDFILE_MAX_IOV - 1; ++i, ++veclen) {
        vec[veclen].iov_base = iov[i].base;
        vec[veclen].iov_len = iov[i].len;
    }
    vec[veclen].iov_base = extra_base;
    vec[veclen].iov_len = extra_len;
    ++veclen;
    return veclen;
}

#define TH_SOCKET_SENDFILE_BUFFERED_MAX (8 * 1024)

/* Read a chunk of the file into a stack buffer, then send header +
 * buffer in one sendmsg. The chunk is capped at
 * TH_SOCKET_SENDFILE_BUFFERED_MAX regardless of len - th_sendfile_op
 * drives further chunks via its own retry loop. */
TH_LOCAL(th_err)
th_socket_ops_os_sendfile(void* self, int fd, const th_iov* iov, size_t iovcnt, th_file* file, size_t offset, size_t len, size_t* result)
{
    (void)self;
    uint8_t buffer[TH_SOCKET_SENDFILE_BUFFERED_MAX];
    size_t toread = TH_MIN(sizeof(buffer), len);
    ssize_t readlen = pread(file->fd, buffer, toread, (off_t)offset);
    if (readlen < 0)
        return TH_ERR_SYSTEM(errno);

    struct iovec vec[TH_SOCKET_SENDFILE_MAX_IOV];
    size_t veclen = th_socket_build_sendfile_iov(vec, iov, iovcnt, buffer, (size_t)readlen);

    int flags = 0;
#if defined(MSG_NOSIGNAL)
    flags |= MSG_NOSIGNAL;
#endif
    struct msghdr msg = {0};
    msg.msg_iov = vec;
#if defined(TH_CONFIG_OS_OSX)
    TH_ASSERT(veclen <= INT_MAX);
    msg.msg_iovlen = (int)veclen;
#else
    msg.msg_iovlen = veclen;
#endif
    ssize_t ret = sendmsg(fd, &msg, flags);
    if (ret < 0)
        return TH_ERR_SYSTEM(errno);
    *result = (size_t)ret;
    return TH_ERR_OK;
}

TH_PRIVATE(th_socket_ops*)
th_socket_ops_os(void)
{
    static th_socket_ops ops = {
        .send = th_socket_ops_os_send,
        .sendvec = th_socket_ops_os_sendvec,
        .recv = th_socket_ops_os_recv,
        .sendfile = th_socket_ops_os_sendfile,
    };
    return &ops;
}

#endif /* TH_CONFIG_OS_POSIX */

TH_PRIVATE(void)
th_socket_init(th_socket* socket, th_loop* loop, th_socket_ops* ops)
{
    socket->loop = loop;
    socket->handle = NULL;
    socket->ops = ops;
}

TH_PRIVATE(th_err)
th_socket_set_fd(th_socket* socket, int fd)
{
    th_socket_close(socket);
    th_err err = th_reactor_create_handle(socket->loop->reactor, &socket->handle, fd);
    if (err != TH_ERR_OK)
        return err;
    th_handle_enable_timeout(socket->handle, true);
    return TH_ERR_OK;
}

TH_PRIVATE(void)
th_socket_close(th_socket* socket)
{
    if (socket->handle) {
        th_handle_destroy(socket->handle);
        socket->handle = NULL;
    }
}

TH_PRIVATE(void)
th_socket_deinit(th_socket* socket)
{
    th_socket_close(socket);
}
