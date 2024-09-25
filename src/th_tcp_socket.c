#include "th_tcp_socket.h"
#include "th_io_task.h"
#include "th_system_error.h"

#if defined(TH_CONFIG_OS_POSIX)
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>
#elif defined(TH_CONFIG_OS_WIN)
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

/* th_tcp_socket functions begin */

#if defined(TH_CONFIG_OS_POSIX)
TH_LOCAL(void)
th_tcp_socket_set_fd_options(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1
        || fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        TH_LOG_ERROR("Failed to set non-blocking: %s", th_strerror(TH_ERR_SYSTEM(errno)));
    }
    int optval = 1;
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval)) == -1)
        TH_LOG_ERROR("Failed to disable nagle: %s", th_strerror(TH_ERR_SYSTEM(errno)));
    if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) == -1)
        TH_LOG_ERROR("Failed to enable keepalive: %s", th_strerror(TH_ERR_SYSTEM(errno)));
}
#elif defined(TH_CONFIG_OS_WIN)
TH_LOCAL(void)
th_tcp_socket_set_fd_options(int fd)
{
    u_long mode = 1;
    if (ioctlsocket(fd, FIONBIO, &mode) == SOCKET_ERROR)
        TH_LOG_ERROR("Failed to set non-blocking: %s", th_strerror(TH_ERR_SYSTEM(WSAGetLastError())));
    int optval = 1;
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&optval, sizeof(optval)) == SOCKET_ERROR)
        TH_LOG_ERROR("Failed to disable nagle: %s", th_strerror(TH_ERR_SYSTEM(WSAGetLastError())));
    if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (const char*)&optval, sizeof(optval)) == SOCKET_ERROR)
        TH_LOG_ERROR("Failed to enable keepalive: %s", th_strerror(TH_ERR_SYSTEM(WSAGetLastError())));
}
#else
TH_LOCAL(void)
th_tcp_socket_set_fd_options(int fd)
{
    (void)fd;
}
#endif

TH_LOCAL(void)
th_tcp_socket_set_fd_impl(void* self, int fd)
{
    th_tcp_socket* sock = self;
    if (sock->handle) {
        th_io_handle_destroy(sock->handle);
        sock->handle = NULL;
    }
    th_tcp_socket_set_fd_options(fd);
    th_context_create_handle(sock->context, &sock->handle, fd);
    th_io_handle_enable_timeout(sock->handle, true);
}

TH_LOCAL(void)
th_tcp_socket_cancel_impl(void* self)
{
    th_tcp_socket* sock = self;
    if (sock->handle)
        th_io_handle_cancel(sock->handle);
}

TH_LOCAL(th_allocator*)
th_tcp_socket_get_allocator_impl(void* self)
{
    th_tcp_socket* sock = self;
    return sock->allocator;
}

TH_LOCAL(th_context*)
th_tcp_socket_get_context_impl(void* self)
{
    th_tcp_socket* sock = self;
    return sock->context;
}

TH_LOCAL(void)
th_tcp_socket_async_write_impl(void* self, void* addr, size_t len, th_io_handler* handler)
{
    th_tcp_socket* sock = self;
    th_io_task* iot = th_io_task_create(sock->allocator);
    if (!iot) {
        th_context_dispatch_handler(sock->context, handler, 0, TH_ERR_BAD_ALLOC);
        return;
    }
    th_io_task_prepare_send(iot, th_io_handle_get_fd(sock->handle), addr, len, handler);
    th_io_handle_submit(sock->handle, iot);
}

TH_LOCAL(void)
th_tcp_socket_async_writev_impl(void* self, th_iov* iov, size_t iovcnt, th_io_handler* handler)
{
    th_tcp_socket* sock = self;
    th_io_task* iot = th_io_task_create(sock->allocator);
    if (!iot) {
        th_context_dispatch_handler(sock->context, handler, 0, TH_ERR_BAD_ALLOC);
        return;
    }
    th_io_task_prepare_sendv(iot, th_io_handle_get_fd(sock->handle), iov, iovcnt, handler);
    th_io_handle_submit(sock->handle, iot);
}

TH_LOCAL(void)
th_tcp_socket_async_read_impl(void* self, void* addr, size_t len, th_io_handler* handler)
{
    th_tcp_socket* sock = self;
    th_io_task* iot = th_io_task_create(sock->allocator);
    if (!iot) {
        th_context_dispatch_handler(sock->context, handler, 0, TH_ERR_BAD_ALLOC);
        return;
    }
    th_io_task_prepare_read(iot, th_io_handle_get_fd(sock->handle), addr, len, handler);
    th_io_handle_submit(sock->handle, iot);
}

TH_LOCAL(void)
th_tcp_socket_async_readv_impl(void* self, th_iov* iov, size_t iovcnt, th_io_handler* handler)
{
    th_tcp_socket* sock = self;
    th_io_task* iot = th_io_task_create(sock->allocator);
    if (!iot) {
        th_context_dispatch_handler(sock->context, handler, 0, TH_ERR_BAD_ALLOC);
        return;
    }
    th_io_task_prepare_readv(iot, th_io_handle_get_fd(sock->handle), iov, iovcnt, handler);
    th_io_handle_submit(sock->handle, iot);
}

TH_LOCAL(void)
th_tcp_socket_async_sendfile_impl(void* self, th_iov* iov, size_t iovcnt, th_file* stream, size_t offset, size_t len, th_io_handler* handler)
{
    th_tcp_socket* sock = self;
    th_io_task* iot = th_io_task_create(sock->allocator);
    if (!iot) {
        th_context_dispatch_handler(sock->context, handler, 0, TH_ERR_BAD_ALLOC);
        return;
    }
    th_io_task_prepare_sendfile(iot, stream, th_io_handle_get_fd(sock->handle), iov, iovcnt, offset, len, handler);
    th_io_handle_submit(sock->handle, iot);
}

TH_PRIVATE(void)
th_tcp_socket_init(th_tcp_socket* sock, th_context* context, th_allocator* allocator)
{
    sock->handle = NULL;
    sock->context = context;
    sock->allocator = allocator ? allocator : th_default_allocator_get();
    th_socket* base = &sock->base;
    base->async_read = th_tcp_socket_async_read_impl;
    base->async_readv = th_tcp_socket_async_readv_impl;
    base->async_write = th_tcp_socket_async_write_impl;
    base->async_writev = th_tcp_socket_async_writev_impl;
    base->async_sendfile = th_tcp_socket_async_sendfile_impl;
    base->set_fd = th_tcp_socket_set_fd_impl;
    base->cancel = th_tcp_socket_cancel_impl;
    base->get_allocator = th_tcp_socket_get_allocator_impl;
    base->get_context = th_tcp_socket_get_context_impl;
}

TH_PRIVATE(void)
th_tcp_socket_close(th_tcp_socket* sock)
{
    if (sock->handle) {
        th_io_handle_destroy(sock->handle);
        sock->handle = NULL;
    }
}

TH_PRIVATE(void)
th_tcp_socket_deinit(th_tcp_socket* sock)
{
    if (sock->handle)
        th_tcp_socket_close(sock);
}

/* th_socket functions end */
