#include "th_poll.h"

#if !defined(TH_CONFIG_OS_WIN)
#include "th_hashmap.h"
#include "th_log.h"
#include "th_system_error.h"
#include "th_timer.h"
#include "th_utility.h"
#include "th_vec.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>

#undef TH_LOG_TAG
#define TH_LOG_TAG "poll"

/* th_pollops_os begin */

TH_LOCAL(int)
th_pollops_os_poll(void* self, struct pollfd* fds, nfds_t nfds, int timeout_ms)
{
    (void)self;
    return poll(fds, nfds, timeout_ms);
}

TH_PRIVATE(th_pollops*)
th_pollops_os(void)
{
    static th_pollops ops = {
        .poll = th_pollops_os_poll,
    };
    return &ops;
}

/* th_pollops_os end */
/* Forward declarations begin */

typedef struct th_poll_reactor th_poll_reactor;
typedef struct th_poll_handle th_poll_handle;
typedef struct th_poll_handle_map th_poll_handle_map;

/* Forward declarations end */
/* th_poll_fd_to_idx_map begin */

TH_INLINE(uint32_t)
th_poll_fd_hash(int fd)
{
    return (uint32_t)fd;
}

TH_INLINE(bool)
th_poll_fd_eq(int a, int b)
{
    return a == b;
}

TH_DEFINE_HASHMAP(th_poll_fd_to_idx_map, int, size_t, th_poll_fd_hash, th_poll_fd_eq, -1)

/* th_poll_fd_to_idx_map end */
/* th_poll_handle begin */

struct th_poll_handle {
    th_handle base;
    th_timer timer;
    th_poll_handle* next;
    th_poll_handle* prev;
    th_allocator* allocator;
    th_poll_reactor* reactor;
    th_op* pending[TH_OP_MAX];
    int fd;
    bool timeout_enabled;
};

TH_DEFINE_OBJ_POOL_ALLOCATOR(th_poll_handle_pool, th_poll_handle, prev, next)
TH_DEFINE_VEC(th_pollfd_vec, struct pollfd, (void))

/* th_poll_handle end */
/* th_poll_handle_map begin */

struct th_poll_handle_map {
    th_poll_fd_to_idx_map fd_to_idx_map;
    th_allocator* allocator;
    th_poll_handle** handles;
    size_t size;
    size_t capacity;
};

TH_LOCAL(void)
th_poll_handle_map_init(th_poll_handle_map* map, th_allocator* allocator)
{
    th_poll_fd_to_idx_map_init(&map->fd_to_idx_map, allocator);
    map->allocator = allocator;
    map->handles = NULL;
    map->size = 0;
    map->capacity = 0;
}

TH_LOCAL(void)
th_poll_handle_map_deinit(th_poll_handle_map* map)
{
    th_poll_fd_to_idx_map_deinit(&map->fd_to_idx_map);
    th_allocator_free(map->allocator, map->handles);
}

TH_LOCAL(void)
th_poll_handle_map_set(th_poll_handle_map* map, int fd, th_poll_handle* handle)
{
    size_t idx = 0;
    th_poll_fd_to_idx_map_iter iter = th_poll_fd_to_idx_map_find(&map->fd_to_idx_map, fd);
    if (iter == NULL) {
        if (map->size == map->capacity) {
            size_t new_capacity = (map->capacity == 0) ? 16 : map->capacity * 2;
            th_poll_handle** new_handles = th_allocator_realloc(map->allocator, map->handles, new_capacity * sizeof(th_poll_handle*));
            if (!new_handles) {
                return;
            }
            map->handles = new_handles;
            map->capacity = new_capacity;
        }
        idx = map->size++;
        th_poll_fd_to_idx_map_set(&map->fd_to_idx_map, fd, idx);
    } else {
        idx = iter->value;
    }
    map->handles[idx] = handle;
}

TH_LOCAL(th_poll_handle*)
th_poll_handle_map_try_get(th_poll_handle_map* map, int fd)
{
    th_poll_handle* handle = NULL;
    th_poll_fd_to_idx_map_iter iter = th_poll_fd_to_idx_map_find(&map->fd_to_idx_map, fd);
    if (iter) {
        handle = map->handles[iter->value];
    }
    return handle;
}

TH_LOCAL(void)
th_poll_handle_map_remove(th_poll_handle_map* map, int fd)
{
    th_poll_fd_to_idx_map_iter iter = th_poll_fd_to_idx_map_find(&map->fd_to_idx_map, fd);
    TH_ASSERT(iter && "Must not remove a non-existent handle");
    if (iter) {
        size_t idx = iter->value;
        th_poll_fd_to_idx_map_erase(&map->fd_to_idx_map, iter);
        if (idx != map->size - 1) {
            th_poll_fd_to_idx_map_iter last = th_poll_fd_to_idx_map_find(&map->fd_to_idx_map, map->handles[map->size - 1]->fd);
            last->value = idx;
            map->handles[idx] = map->handles[map->size - 1];
        }
        --map->size;
    }
}

/* th_poll_handle_map implementation end */
/* th_poll_reactor begin */

struct th_poll_reactor {
    th_reactor base;
    th_loop* loop;
    th_allocator* allocator;
    th_clock* clock;
    th_pollops* ops;
    th_poll_handle_pool handle_allocator;
    th_poll_handle_map handles;
    th_pollfd_vec fds;
};

/* th_poll_reactor end */
/* th_poll_handle implementation begin */

TH_LOCAL(th_err)
th_poll_handle_submit(void* self, th_op* op)
{
    th_poll_handle* handle = (th_poll_handle*)self;
    th_poll_reactor* reactor = handle->reactor;
    TH_ASSERT(handle->pending[op->type] == NULL && "Handle already has a pending op for this op type");
    if (th_op_get_flags(op) & TH_OP_IMMEDIATE) {
        th_op_perform(op);
        return TH_ERR_OK;
    }
    handle->pending[op->type] = op;
    struct pollfd pfd = {.fd = handle->fd, .events = (op->type == TH_OP_READ) ? POLLIN : POLLOUT};
    if (handle->timeout_enabled) {
        th_timer_set(&handle->timer, th_seconds(TH_CONFIG_IO_TIMEOUT));
    }
    th_err err = TH_ERR_OK;
    if ((err = th_pollfd_vec_push_back(&reactor->fds, pfd)) != TH_ERR_OK) {
        handle->pending[op->type] = NULL;
        return err;
    }
    th_loop_increase_task_count(reactor->loop);
    return TH_ERR_OK;
}

TH_LOCAL(void)
th_poll_handle_cancel(void* self)
{
    th_poll_handle* handle = (th_poll_handle*)self;
    for (int i = 0; i < TH_OP_MAX; ++i) {
        th_op* op = handle->pending[i];
        if (op) {
            handle->pending[i] = NULL;
            th_op_abort(op, TH_ERR_SYSTEM(TH_ECANCELED));
            th_loop_decrease_task_count(handle->reactor->loop);
        }
    }
}

TH_LOCAL(int)
th_poll_handle_get_fd(const void* self)
{
    const th_poll_handle* handle = (const th_poll_handle*)self;
    return handle->fd;
}

TH_LOCAL(void)
th_poll_handle_enable_timeout(void* self, bool enable)
{
    th_poll_handle* handle = (th_poll_handle*)self;
    handle->timeout_enabled = enable;
}

TH_LOCAL(void)
th_poll_handle_destroy(void* self)
{
    th_poll_handle* handle = (th_poll_handle*)self;
    th_poll_handle_map_remove(&handle->reactor->handles, handle->fd);
    close(handle->fd);
    th_allocator_free(handle->allocator, handle);
}

static const th_handle_methods th_poll_handle_methods = {
    .cancel = th_poll_handle_cancel,
    .submit = th_poll_handle_submit,
    .enable_timeout = th_poll_handle_enable_timeout,
    .get_fd = th_poll_handle_get_fd,
    .destroy = th_poll_handle_destroy,
};

TH_LOCAL(void)
th_poll_handle_init(th_poll_handle* handle, th_poll_reactor* reactor, int fd, th_allocator* allocator)
{
    handle->base.methods = &th_poll_handle_methods;
    th_timer_init(&handle->timer, reactor->clock);
    handle->pending[TH_OP_READ] = NULL;
    handle->pending[TH_OP_WRITE] = NULL;
    handle->allocator = allocator;
    handle->reactor = reactor;
    handle->fd = fd;
    handle->timeout_enabled = false;
}

/* th_poll_handle implementation end */
/* th_poll_reactor implementation begin */

TH_LOCAL(th_err)
th_poll_reactor_create_handle(void* self, th_handle** out, int fd)
{
    th_poll_reactor* reactor = (th_poll_reactor*)self;
    th_poll_handle* handle = th_poll_handle_pool_alloc(&reactor->handle_allocator, sizeof(th_poll_handle));
    if (!handle) {
        return TH_ERR_BAD_ALLOC;
    }
    th_poll_handle_init(handle, reactor, fd, &reactor->handle_allocator.base);
    th_poll_handle_map_set(&reactor->handles, handle->fd, handle);
    *out = (th_handle*)handle;
    return TH_ERR_OK;
}

TH_LOCAL(void)
th_poll_reactor_run(void* self, int timeout_ms)
{
    th_poll_reactor* reactor = (th_poll_reactor*)self;
    nfds_t nfds = (nfds_t)th_pollfd_vec_size(&reactor->fds);
    int ret = reactor->ops->poll(reactor->ops, th_pollfd_vec_begin(&reactor->fds), nfds, timeout_ms);
    if (ret == -1) {
        TH_LOG_WARN("poll failed: %s", strerror(errno));
        return;
    }

    size_t reenqueue = 0;
    for (size_t i = 0; i < nfds; ++i) {
        struct pollfd* pfd = th_pollfd_vec_at(&reactor->fds, i);
        th_poll_handle* handle = th_poll_handle_map_try_get(&reactor->handles, pfd->fd);
        if (!handle) // handle was removed
            continue;
        short revents = pfd->revents;
        th_op_type type = (pfd->events & POLLIN) ? TH_OP_READ : TH_OP_WRITE;
        th_op* op = handle->pending[type];
        if (revents && op) {
            handle->pending[type] = NULL;
            th_loop_decrease_task_count(reactor->loop);
            if (revents & pfd->events) {
                th_op_perform(op);
            } else if (revents & POLLHUP) {
                th_op_abort(op, TH_ERR_EOF);
            } else if (revents & (POLLERR | POLLPRI)) {
                th_op_abort(op, TH_ERR_SYSTEM(TH_EIO));
            } else if (revents & POLLNVAL) {
                th_op_abort(op, TH_ERR_SYSTEM(TH_EBADF));
            } else {
                TH_LOG_ERROR("Unknown poll event: %d", revents);
                th_op_abort(op, TH_ERR_UNKNOWN);
            }
        } else if (op) { // reenqueue
            if (handle->timeout_enabled && th_timer_expired(&handle->timer)) {
                handle->pending[type] = NULL;
                th_loop_decrease_task_count(reactor->loop);
                th_op_abort(op, TH_ERR_SYSTEM(TH_ETIMEDOUT));
            } else {
                if (reenqueue < i)
                    *th_pollfd_vec_at(&reactor->fds, reenqueue) = *pfd;
                ++reenqueue;
            }
        }
        // handles without a pending op were cancelled, don't reenqueue
    }
    /* th_op_perform above may have synchronously resubmitted an op,
     * pushing a new pollfd past index nfds (the size we polled on).
     * Those entries must survive the compaction below, not just the
     * ones inside [0, nfds). */
    size_t total = th_pollfd_vec_size(&reactor->fds);
    for (size_t i = nfds; i < total; ++i, ++reenqueue) {
        if (reenqueue < i)
            *th_pollfd_vec_at(&reactor->fds, reenqueue) = *th_pollfd_vec_at(&reactor->fds, i);
    }
    th_pollfd_vec_resize(&reactor->fds, reenqueue);
}

TH_LOCAL(void)
th_poll_reactor_deinit(th_poll_reactor* reactor)
{
    th_poll_handle_map_deinit(&reactor->handles);
    th_poll_handle_pool_deinit(&reactor->handle_allocator);
    th_pollfd_vec_deinit(&reactor->fds);
}

TH_LOCAL(void)
th_poll_reactor_destroy(void* self)
{
    th_poll_reactor* reactor = (th_poll_reactor*)self;
    th_allocator* allocator = reactor->allocator;
    th_poll_reactor_deinit(reactor);
    th_allocator_free(allocator, reactor);
}

static const th_reactor_methods th_poll_reactor_methods = {
    .run = th_poll_reactor_run,
    .create_handle = th_poll_reactor_create_handle,
    .destroy = th_poll_reactor_destroy,
};

TH_LOCAL(void)
th_poll_reactor_init(th_poll_reactor* reactor, th_loop* loop, th_allocator* allocator, th_clock* clock, th_pollops* ops)
{
    reactor->base.methods = &th_poll_reactor_methods;
    reactor->loop = loop;
    reactor->allocator = allocator;
    reactor->clock = clock;
    reactor->ops = ops;
    th_pollfd_vec_init(&reactor->fds, allocator);
    th_poll_handle_map_init(&reactor->handles, allocator);
    th_poll_handle_pool_init(&reactor->handle_allocator, allocator, 16, 8 * 1024);
}

TH_PRIVATE(th_err)
th_poll_create(th_reactor** out, th_loop* loop, th_allocator* allocator, th_clock* clock, th_pollops* ops)
{
    allocator = allocator ? allocator : th_default_allocator_get();
    th_poll_reactor* reactor = th_allocator_alloc(allocator, sizeof(th_poll_reactor));
    if (!reactor) {
        return TH_ERR_BAD_ALLOC;
    }
    th_poll_reactor_init(reactor, loop, allocator, clock, ops);
    *out = &reactor->base;
    return TH_ERR_OK;
}

/* th_poll_reactor implementation end */

#endif /* !TH_CONFIG_OS_WIN */
