#ifndef TH_SOCKET_H
#define TH_SOCKET_H

#include <th.h>

#include <sys/socket.h>

#include "th_context.h"
#include "th_file.h"
#include "th_io_service.h"
#include "th_utility.h"

typedef struct th_address {
    struct sockaddr_storage addr;
    socklen_t addrlen;
} th_address;

TH_PRIVATE(void)
th_address_init(th_address* addr);

/* th_socket_handler begin */

typedef th_io_handler th_socket_handler;
#define th_socket_handler_init th_io_handler_init
#define th_socket_handler_complete th_io_handler_complete

/* th_socket_task_handler end */
/* th_socket begin */

typedef struct th_socket_methods {
    void (*set_fd)(void* self, int fd);
    void (*cancel)(void* self);
    th_allocator* (*get_allocator)(void* self);
    th_context* (*get_context)(void* self);
    void (*async_write)(void* self, void* addr, size_t len, th_socket_handler* handler);
    void (*async_writev)(void* self, th_iov* iov, size_t len, th_socket_handler* handler);
    void (*async_read)(void* self, void* addr, size_t len, th_socket_handler* handler);
    void (*async_readv)(void* self, th_iov* iov, size_t len, th_socket_handler* handler);
    void (*async_sendfile)(void* self, th_iov* header, size_t iovcnt,
                           th_file* stream, size_t offset, size_t len, th_socket_handler* handler);
} th_socket_methods;

typedef struct th_socket {
    const th_socket_methods* methods;
} th_socket;

TH_INLINE(void)
th_socket_set_fd(th_socket* socket, int fd)
{
    socket->methods->set_fd(socket, fd);
}

TH_INLINE(void)
th_socket_cancel(th_socket* socket)
{
    socket->methods->cancel(socket);
}

TH_INLINE(th_allocator*)
th_socket_get_allocator(th_socket* socket)
{
    return socket->methods->get_allocator(socket);
}

TH_INLINE(th_context*)
th_socket_get_context(th_socket* socket)
{
    return socket->methods->get_context(socket);
}

TH_INLINE(void)
th_socket_async_write(th_socket* sock, void* addr, size_t len, th_socket_handler* handler)
{
    sock->methods->async_write(sock, addr, len, handler);
}

TH_INLINE(void)
th_socket_async_writev(th_socket* sock, th_iov* iov, size_t len, th_socket_handler* handler)
{
    sock->methods->async_writev(sock, iov, len, handler);
}

TH_INLINE(void)
th_socket_async_read(th_socket* sock, void* addr, size_t len, th_socket_handler* handler)
{
    sock->methods->async_read(sock, addr, len, handler);
}

TH_INLINE(void)
th_socket_async_readv(th_socket* sock, th_iov* iov, size_t len, th_socket_handler* handler)
{
    sock->methods->async_readv(sock, iov, len, handler);
}

TH_INLINE(void)
th_socket_async_sendfile(th_socket* sock, th_iov* header, size_t iovcnt,
                         th_file* stream, size_t offset, size_t len, th_socket_handler* handler)
{
    sock->methods->async_sendfile(sock, header, iovcnt, stream, offset, len, handler);
}

/* th_socket end */
/** generic socket functions begin */

TH_PRIVATE(void)
th_socket_async_write_exact(th_socket* sock, void* addr, size_t len, th_socket_handler* handler) TH_MAYBE_UNUSED;

TH_PRIVATE(void)
th_socket_async_writev_exact(th_socket* sock, th_iov* iov, size_t len, th_socket_handler* handler);

TH_PRIVATE(void)
th_socket_async_read_exact(th_socket* sock, void* addr, size_t len, th_socket_handler* handler);

TH_PRIVATE(void)
th_socket_async_readv_exact(th_socket* sock, th_iov* iov, size_t len, th_socket_handler* handler) TH_MAYBE_UNUSED;

TH_PRIVATE(void)
th_socket_async_sendfile_exact(th_socket* sock, th_iov* iov, size_t iovcnt, th_file* stream, size_t offset, size_t len, th_socket_handler* handler);

/* th_socket functionss end */

#endif
