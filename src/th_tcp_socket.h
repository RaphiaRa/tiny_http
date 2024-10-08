#ifndef TH_TCP_SOCKET_H
#define TH_TCP_SOCKET_H

#include "th_socket.h"

/* th_tcp_socket begin */

typedef struct th_tcp_socket {
    th_socket base;
    th_context* context;
    th_allocator* allocator;
    th_io_handle* handle;
} th_tcp_socket;

TH_PRIVATE(void)
th_tcp_socket_init(th_tcp_socket* socket, th_context* context, th_allocator* allocator);

/** th_socket_close
 * @brief Closes the underlying file descriptor of the socket.
 * while the socket object is still valid and can be reused.
 */
TH_PRIVATE(void)
th_tcp_socket_close(th_tcp_socket* socket);

TH_PRIVATE(void)
th_tcp_socket_deinit(th_tcp_socket* socket);

#define th_tcp_socket_set_fd(socket, fd) ((socket)->base.methods->set_fd((socket), (fd)))

#define th_tcp_socket_cancel(socket) ((socket)->base.methods->cancel((socket)))

#define th_tcp_socket_get_allocator(socket) ((socket)->base.methods->get_allocator((socket)))

#define th_tcp_socket_get_context(socket) ((socket)->base.methods->get_context((socket)))

#define th_tcp_socket_async_write(socket, addr, len, handler) ((socket)->base.methods->async_write((socket), (addr), (len), (handler)))

#define th_tcp_socket_async_writev(socket, iov, iovcnt, handler) ((socket)->base.methods->async_writev((socket), (iov), (iovcnt), (handler)))

#define th_tcp_socket_async_read(socket, addr, len, handler) ((socket)->base.methods->async_read((socket), (addr), (len), (handler)))

#define th_tcp_socket_async_readv(socket, iov, iovcnt, handler) ((socket)->base.methods->async_readv((socket), (iov), (iovcnt), (handler)))

#define th_tcp_socket_async_sendfile(socket, header, iovcnt, stream, offset, len, handler) ((socket)->base.methods->async_sendfile((socket), (header), (iovcnt), (stream), (offset), (len), (handler)))

/* th_tcp_socket end */

#endif
