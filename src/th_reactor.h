#ifndef TH_REACTOR_H
#define TH_REACTOR_H

#include <th.h>

#include "th_config.h"
#include "th_op.h"

/** th_handle
 * @brief One fd registered with a th_reactor. Vtable so different reactor
 * backends (poll, kqueue, ...) can implement it without the caller caring.
 */
typedef struct th_handle_methods {
    void (*cancel)(void* self);
    th_err (*submit)(void* self, th_op* op);
    void (*enable_timeout)(void* self, bool enabled);
    int (*get_fd)(const void* self);
    void (*destroy)(void* self);
} th_handle_methods;

typedef struct th_handle {
    const th_handle_methods* methods;
} th_handle;

TH_INLINE(void)
th_handle_cancel(th_handle* handle)
{
    handle->methods->cancel(handle);
}

/** th_handle_submit
 * @brief If op is still TH_OP_IMMEDIATE (its very first attempt), runs
 * op->base.fn inline right now — an op that's immediately satisfiable
 * completes without ever touching the reactor. Otherwise (a resubmit
 * after TH_EAGAIN/TH_EWOULDBLOCK, where TH_OP_IMMEDIATE is already
 * clear) skips straight to waiting for op->type readiness on this
 * handle's fd and runs fn once ready. At most one op per op type may be
 * pending at a time.
 */
TH_INLINE(th_err)
th_handle_submit(th_handle* handle, th_op* op)
{
    return handle->methods->submit(handle, op);
}

TH_INLINE(int)
th_handle_get_fd(const th_handle* handle)
{
    return handle->methods->get_fd(handle);
}

TH_INLINE(void)
th_handle_enable_timeout(th_handle* handle, bool enabled)
{
    handle->methods->enable_timeout(handle, enabled);
}

TH_INLINE(void)
th_handle_destroy(th_handle* handle)
{
    handle->methods->destroy(handle);
}

/** th_reactor
 * @brief Event loop backend: turns fd readiness into op completions.
 */
typedef struct th_reactor_methods {
    void (*run)(void* self, int timeout_ms);
    th_err (*create_handle)(void* self, th_handle** out, int fd);
    void (*destroy)(void* self);
} th_reactor_methods;

typedef struct th_reactor {
    const th_reactor_methods* methods;
} th_reactor;

TH_INLINE(void)
th_reactor_run(th_reactor* reactor, int timeout_ms)
{
    reactor->methods->run(reactor, timeout_ms);
}

TH_INLINE(th_err)
th_reactor_create_handle(th_reactor* reactor, th_handle** out, int fd)
{
    return reactor->methods->create_handle(reactor, out, fd);
}

TH_INLINE(void)
th_reactor_destroy(th_reactor* reactor)
{
    if (reactor->methods->destroy)
        reactor->methods->destroy(reactor);
}

#endif
