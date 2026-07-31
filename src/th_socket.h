#ifndef TH_SOCKET_H
#define TH_SOCKET_H

#include <th.h>

#include "th_file.h"
#include "th_iov.h"
#include "th_loop.h"
#include "th_reactor.h"

#include <stddef.h>

/** th_socket_ops
 * @brief The raw send/recv syscalls a th_socket performs. Injected at
 * construction time so tests can fake a socket without a real fd. Each
 * call behaves like the underlying syscall: TH_ERR_SYSTEM(TH_EAGAIN) /
 * TH_ERR_SYSTEM(TH_EWOULDBLOCK) when it would block, otherwise TH_ERR_OK
 * with *result set to the number of bytes transferred.
 */
typedef struct th_socket_ops {
    th_err (*send)(void* self, int fd, const void* addr, size_t len, size_t* result);
    th_err (*sendvec)(void* self, int fd, const th_iov* iov, size_t iovcnt, size_t* result);
    th_err (*recv)(void* self, int fd, void* addr, size_t len, size_t* result);

    /** sendfile
     * @brief Sends header (iov/iovcnt, may be empty) followed by up to
     * len bytes of file starting at offset. *result is the total bytes
     * transferred across header and file combined.
     */
    th_err (*sendfile)(void* self, int fd, const th_iov* iov, size_t iovcnt, th_file* file, size_t offset, size_t len, size_t* result);
} th_socket_ops;

TH_PRIVATE(th_socket_ops*)
th_socket_ops_os(void);

/** th_socket
 * @brief A non-blocking TCP connection: an fd registered with a reactor
 * plus the ops used to read/write it. Holds the th_loop (not just its
 * reactor) so ops can defer completion via th_socket_post instead of
 * invoking it inline.
 */
typedef struct th_socket {
    th_loop* loop;
    th_handle* handle;
    th_socket_ops* ops;
} th_socket;

TH_PRIVATE(void)
th_socket_init(th_socket* socket, th_loop* loop, th_socket_ops* ops);

/** th_socket_set_fd
 * @brief Registers fd with the socket's reactor, replacing any fd
 * previously set.
 */
TH_PRIVATE(th_err)
th_socket_set_fd(th_socket* socket, int fd);

TH_INLINE(int)
th_socket_get_fd(const th_socket* socket)
{
    return socket->handle ? th_handle_get_fd(socket->handle) : -1;
}

TH_INLINE(void)
th_socket_cancel(th_socket* socket)
{
    if (socket->handle)
        th_handle_cancel(socket->handle);
}

TH_INLINE(void)
th_socket_enable_timeout(th_socket* socket, bool enabled)
{
    th_handle_enable_timeout(socket->handle, enabled);
}

/** th_socket_submit
 * @brief Waits for op->type readiness on the socket's fd, then runs op.
 */
TH_INLINE(th_err)
th_socket_submit(th_socket* socket, th_op* op)
{
    return th_handle_submit(socket->handle, op);
}

/** th_socket_post
 * @brief Queues task (typically an op with TH_OP_COMPLETED just set) to
 * finalize on a future th_loop_poll/th_loop_run, rather than inline —
 * bounds stack depth when I/O completes immediately, repeatedly.
 */
TH_INLINE(void)
th_socket_post(th_socket* socket, th_task* task)
{
    th_loop_push_task(socket->loop, task);
}

TH_INLINE(th_err)
th_socket_send(th_socket* socket, const void* addr, size_t len, size_t* result)
{
    return socket->ops->send(socket->ops, th_socket_get_fd(socket), addr, len, result);
}

TH_INLINE(th_err)
th_socket_sendvec(th_socket* socket, const th_iov* iov, size_t iovcnt, size_t* result)
{
    return socket->ops->sendvec(socket->ops, th_socket_get_fd(socket), iov, iovcnt, result);
}

TH_INLINE(th_err)
th_socket_recv(th_socket* socket, void* addr, size_t len, size_t* result)
{
    return socket->ops->recv(socket->ops, th_socket_get_fd(socket), addr, len, result);
}

TH_INLINE(th_err)
th_socket_sendfile(th_socket* socket, const th_iov* iov, size_t iovcnt, th_file* file, size_t offset, size_t len, size_t* result)
{
    return socket->ops->sendfile(socket->ops, th_socket_get_fd(socket), iov, iovcnt, file, offset, len, result);
}

/** th_socket_close
 * @brief Closes the underlying fd; the socket object itself stays valid
 * and can be reused via th_socket_set_fd.
 */
TH_PRIVATE(void)
th_socket_close(th_socket* socket);

TH_PRIVATE(void)
th_socket_deinit(th_socket* socket);

#endif
