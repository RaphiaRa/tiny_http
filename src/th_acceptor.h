#ifndef TH_ACCEPTOR_H
#define TH_ACCEPTOR_H

#include <th.h>

#include "th_address.h"
#include "th_loop.h"
#include "th_reactor.h"
#include "th_socket.h"

/** th_acceptor_ops
 * @brief Mirrors the raw listen-socket syscalls directly, one op each,
 * so tests can fake a th_acceptor without a real fd.
 */
typedef struct th_acceptor_ops {
    th_err (*socket)(void* self, int domain, int type, int protocol, int* out_fd);
    th_err (*setsockopt)(void* self, int fd, int level, int optname, const void* optval, socklen_t optlen);
    th_err (*set_nonblocking)(void* self, int fd);
    th_err (*bind)(void* self, int fd, const struct sockaddr* addr, socklen_t addrlen);
    th_err (*listen)(void* self, int fd, int backlog);
    void (*close)(void* self, int fd);
    // TH_ERR_SYSTEM(TH_EAGAIN)/TH_EWOULDBLOCK when nothing is pending.
    th_err (*accept)(void* self, int fd, th_address* addr, int* out_fd);
} th_acceptor_ops;

TH_PRIVATE(th_acceptor_ops*)
th_acceptor_ops_os(void);

/** th_acceptor
 * @brief A non-blocking listening socket: an fd registered with a reactor
 * plus the ops used to open it and accept connections from it. Holds the
 * th_loop (not just its reactor) so th_accept_op can defer completion via
 * th_acceptor_post instead of invoking it inline.
 */
typedef struct th_acceptor {
    th_loop* loop;
    th_handle* handle;
    th_acceptor_ops* ops;
} th_acceptor;

TH_PRIVATE(void)
th_acceptor_init(th_acceptor* acceptor, th_loop* loop, th_acceptor_ops* ops);

/** th_acceptor_open
 * @brief Opens a listening socket matching info and registers it with
 * the acceptor's reactor, replacing any fd previously set.
 */
TH_PRIVATE(th_err)
th_acceptor_open(th_acceptor* acceptor, const th_addrinfo* info);

TH_INLINE(int)
th_acceptor_get_fd(const th_acceptor* acceptor)
{
    return acceptor->handle ? th_handle_get_fd(acceptor->handle) : -1;
}

TH_INLINE(void)
th_acceptor_cancel(th_acceptor* acceptor)
{
    if (acceptor->handle)
        th_handle_cancel(acceptor->handle);
}

/** th_acceptor_submit
 * @brief Waits for op->type readiness on the acceptor's fd, then runs op.
 */
TH_INLINE(th_err)
th_acceptor_submit(th_acceptor* acceptor, th_op* op)
{
    return th_handle_submit(acceptor->handle, op);
}

/** th_acceptor_post
 * @brief Queues task (typically an op with TH_OP_COMPLETED just set) to
 * finalize on a future th_loop_poll/th_loop_run, rather than inline.
 */
TH_INLINE(void)
th_acceptor_post(th_acceptor* acceptor, th_task* task)
{
    th_loop_push_task(acceptor->loop, task);
}

/** th_acceptor_accept
 * @brief Accepts one pending connection and registers it with out_socket
 * (via th_socket_set_fd), replacing any fd previously set on it.
 */
TH_PRIVATE(th_err)
th_acceptor_accept(th_acceptor* acceptor, th_address* addr, th_socket* out_socket);

/** th_acceptor_close
 * @brief Closes the underlying fd; the acceptor object itself stays valid
 * and can be reused via th_acceptor_open.
 */
TH_PRIVATE(void)
th_acceptor_close(th_acceptor* acceptor);

TH_PRIVATE(void)
th_acceptor_deinit(th_acceptor* acceptor);

#endif
