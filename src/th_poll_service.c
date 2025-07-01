#include "th_poll_service.h"

#ifdef TH_CONFIG_WITH_POLL
#include "th_io_op_posix.h"
#include "th_log.h"
#include "th_system_error.h"
#include "th_utility.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>

#undef TH_LOG_TAG
#define TH_LOG_TAG "poll_service"

/* th_poll_handle_map implementation begin */

TH_LOCAL(void)
th_poll_handle_map_init(th_poll_handle_map* map, th_allocator* allocator)
{
    th_fd_to_idx_map_init(&map->fd_to_idx_map, allocator);
    map->allocator = (allocator) ? allocator : th_default_allocator_get();
    map->handles = NULL;
    map->size = 0;
    map->capacity = 0;
}

TH_LOCAL(void)
th_poll_handle_map_deinit(th_poll_handle_map* map)
{
    th_fd_to_idx_map_deinit(&map->fd_to_idx_map);
    th_allocator_free(map->allocator, map->handles);
}

/** th_poll_handle_map_set
 * @brief Sets the poll handle for the given file descriptor.
 */
TH_LOCAL(void)
th_poll_handle_map_set(th_poll_handle_map* map, int fd, th_poll_handle* handle)
{
    size_t idx = 0;
    th_fd_to_idx_map_iter iter = th_fd_to_idx_map_find(&map->fd_to_idx_map, fd);
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
        th_fd_to_idx_map_set(&map->fd_to_idx_map, fd, idx);
    } else {
        idx = iter->value;
    }
    map->handles[idx] = handle;
}

/* th_poll_handle_map_try_get
 * @brief Get the poll handle for the given file descriptor.
 * @param map The handle map.
 * @param fd The file descriptor.
 * @return The poll handle, NULL if the handle wasn't found.
 */
TH_LOCAL(th_poll_handle*)
th_poll_handle_map_try_get(th_poll_handle_map* map, int fd)
{
    th_poll_handle* handle = NULL;
    th_fd_to_idx_map_iter iter = th_fd_to_idx_map_find(&map->fd_to_idx_map, fd);
    if (iter) {
        handle = map->handles[iter->value];
    }
    return handle;
}

TH_LOCAL(void)
th_poll_handle_map_remove(th_poll_handle_map* map, int fd)
{
    th_fd_to_idx_map_iter iter = th_fd_to_idx_map_find(&map->fd_to_idx_map, fd);
    TH_ASSERT(iter && "Must not remove a non-existent handle");
    if (iter) {
        size_t idx = iter->value;
        th_fd_to_idx_map_erase(&map->fd_to_idx_map, iter);
        if (idx != map->size - 1) {
            th_fd_to_idx_map_iter last = th_fd_to_idx_map_find(&map->fd_to_idx_map, map->handles[map->size - 1]->fd);
            last->value = idx;
            map->handles[idx] = map->handles[map->size - 1];
        }
        --map->size;
    }
}

/* th_poll_handle_map implementation end */
/* th_poll_handle implementation begin */

TH_LOCAL(void)
th_poll_handle_submit(void* self, th_io_task* task)
{
    th_poll_handle* handle = (th_poll_handle*)self;
    th_poll_service* service = handle->service;
    th_io_handler* on_complete = th_io_task_try_execute(task);
    if (on_complete) {
        th_runner_push_task(service->runner, (th_task*)on_complete);
        return;
    }
    th_io_op_type op_type = TH_IO_OP_TYPE(task->op);
    handle->iot[op_type - 1] = task;
    struct pollfd pfd = {.fd = handle->fd, .events = 0};
    switch (op_type) {
    case TH_IO_OP_TYPE_READ:
        pfd.events = POLLIN;
        break;
    case TH_IO_OP_TYPE_WRITE:
        pfd.events = POLLOUT;
        break;
    default:
        TH_ASSERT(0 && "Invalid operation");
        break;
    }
    if (handle->timeout_enabled) {
        th_err err = th_timer_set(&handle->timer, th_seconds(TH_CONFIG_IO_TIMEOUT));
        if (err != TH_ERR_OK) {
            TH_LOG_ERROR("Failed to set timer: %s, disabling timeout", th_strerror(err));
            handle->timeout_enabled = false;
        }
    }
    th_err err = TH_ERR_OK;
    if ((err = th_pollfd_vec_push_back(&service->fds, pfd)) != TH_ERR_OK) {
        TH_LOG_ERROR("Failed to push back pollfd");
        th_runner_push_task(service->runner, (th_task*)th_io_task_abort(task, err));
        return;
    }
    th_runner_increase_task_count(service->runner);
}

TH_LOCAL(void)
th_poll_handle_cancel(void* self)
{
    th_poll_handle* handle = (th_poll_handle*)self;
    for (int i = 0; i < TH_IO_OP_TYPE_MAX; ++i) {
        th_io_task* iot = handle->iot[i];
        if (iot) {
            handle->iot[i] = NULL;
            th_runner_push_uncounted_task(handle->service->runner, (th_task*)th_io_task_abort(iot, TH_ERR_SYSTEM(TH_ECANCELED)));
        }
    }
}

TH_LOCAL(int)
th_poll_handle_get_fd(void* self)
{
    th_poll_handle* handle = (th_poll_handle*)self;
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
    th_poll_handle_map_remove(&handle->service->handles, handle->fd);
    close(handle->fd);
    th_allocator_free(handle->allocator, handle);
}

TH_LOCAL(void)
th_poll_handle_init(th_poll_handle* handle, th_poll_service* service, int fd, th_allocator* allocator)
{
    handle->base.submit = th_poll_handle_submit;
    handle->base.cancel = th_poll_handle_cancel;
    handle->base.destroy = th_poll_handle_destroy;
    handle->base.get_fd = th_poll_handle_get_fd;
    handle->base.enable_timeout = th_poll_handle_enable_timeout;
    th_timer_init(&handle->timer);
    handle->iot[TH_IO_OP_TYPE_READ - 1] = NULL;
    handle->iot[TH_IO_OP_TYPE_WRITE - 1] = NULL;
    handle->allocator = allocator;
    handle->service = service;
    handle->fd = fd;
    handle->timeout_enabled = false;
}

/* th_poll_handle implementation end */
/* th_poll_service implementation begin */

TH_LOCAL(th_err)
th_poll_service_create_handle(void* self, th_io_handle** out, int fd)
{
    th_poll_service* service = (th_poll_service*)self;
    th_poll_handle* handle = th_poll_handle_pool_alloc(&service->handle_allocator, sizeof(th_poll_handle));
    if (!handle) {
        return TH_ERR_BAD_ALLOC;
    }
    th_poll_handle_init(handle, service, fd, &service->handle_allocator.base);
    th_poll_handle_map_set(&service->handles, handle->fd, handle);
    *out = (th_io_handle*)handle;
    return TH_ERR_OK;
}

TH_LOCAL(void)
th_poll_service_run(void* self, int timeout_ms)
{
    th_poll_service* service = (th_poll_service*)self;
    nfds_t nfds = (nfds_t)th_pollfd_vec_size(&service->fds);
    int ret = poll(th_pollfd_vec_begin(&service->fds), nfds, timeout_ms);
    if (ret <= 0) {
        if (ret == -1)
            TH_LOG_WARN("poll failed: %s", strerror(errno));
        return;
    }

    size_t reenqueue = 0;
    for (size_t i = 0; i < nfds; ++i) {
        th_poll_handle* handle = th_poll_handle_map_try_get(&service->handles, th_pollfd_vec_at(&service->fds, i)->fd);
        if (!handle) // handle was removed
            continue;
        short revents = th_pollfd_vec_at(&service->fds, i)->revents;
        short events = th_pollfd_vec_at(&service->fds, i)->events & (POLLIN | POLLOUT);
        int op_index = 0;
        switch (events) {
        case POLLIN:
            op_index = TH_IO_OP_TYPE_READ - 1;
            break;
        case POLLOUT:
            op_index = TH_IO_OP_TYPE_WRITE - 1;
            break;
        default:
            TH_LOG_ERROR("Unknown poll event: %d", events);
            continue;
            break;
        }
        th_io_task* iot = handle->iot[op_index];
        if (revents && iot) {
            if (revents & events) {
                th_runner_push_uncounted_task(service->runner, (th_task*)iot);
            } else if (revents & POLLHUP) {
                th_runner_push_uncounted_task(service->runner, (th_task*)th_io_task_abort(iot, TH_ERR_EOF));
            } else if (revents & (POLLERR | POLLPRI)) {
                th_runner_push_uncounted_task(service->runner, (th_task*)th_io_task_abort(iot, TH_ERR_SYSTEM(TH_EIO)));
            } else if (revents & POLLNVAL) {
                th_runner_push_uncounted_task(service->runner, (th_task*)th_io_task_abort(iot, TH_ERR_SYSTEM(TH_EBADF)));
            } else {
                TH_LOG_ERROR("[th_poll_service] Unknown poll event: %d", revents);
                th_runner_push_uncounted_task(service->runner, (th_task*)th_io_task_abort(iot, TH_ERR_UNKNOWN));
            }
            handle->iot[op_index] = NULL;
        } else if (iot) { // reenqueue
            if (handle->timeout_enabled && th_timer_expired(&handle->timer)) {
                th_runner_push_uncounted_task(service->runner, (th_task*)th_io_task_abort(iot, TH_ERR_SYSTEM(TH_ETIMEDOUT)));
                handle->iot[op_index] = NULL;
            } else {
                if (reenqueue < i)
                    *th_pollfd_vec_at(&service->fds, reenqueue) = *th_pollfd_vec_at(&service->fds, i);
                ++reenqueue;
            }
        }
        // handles without iot were cancelled, so we don't need to reenqueue them
    }
    th_pollfd_vec_resize(&service->fds, reenqueue);
    return;
}

TH_LOCAL(void)
th_poll_service_deinit(th_poll_service* service)
{
    th_poll_handle_map_deinit(&service->handles);
    th_poll_handle_pool_deinit(&service->handle_allocator);
    th_pollfd_vec_deinit(&service->fds);
}

TH_LOCAL(void)
th_poll_service_destroy(void* self)
{
    th_poll_service* service = (th_poll_service*)self;
    th_poll_service_deinit(service);
    th_allocator_free(service->allocator, service);
}

TH_LOCAL(th_err)
th_poll_service_init(th_poll_service* service, th_runner* runner, th_allocator* allocator)
{
    service->base.run = th_poll_service_run;
    service->base.destroy = th_poll_service_destroy;
    service->base.create_handle = th_poll_service_create_handle;
    service->allocator = allocator;
    service->runner = runner;
    th_pollfd_vec_init(&service->fds, allocator);
    th_poll_handle_map_init(&service->handles, allocator);
    th_poll_handle_pool_init(&service->handle_allocator, allocator, 16, 8 * 1024);
    return TH_ERR_OK;
}

TH_PRIVATE(th_err)
th_poll_service_create(th_io_service** out, th_runner* runner, th_allocator* allocator)
{
    allocator = allocator ? allocator : th_default_allocator_get();
    th_poll_service* service = (th_poll_service*)th_allocator_alloc(allocator, sizeof(th_poll_service));
    if (!service) {
        return TH_ERR_BAD_ALLOC;
    }
    memset(service, 0, sizeof(th_poll_service));
    th_err err = TH_ERR_OK;
    if ((err = th_poll_service_init(service, runner, allocator)) != TH_ERR_OK) {
        th_allocator_free(allocator, service);
        return err;
    }
    *out = &service->base;
    return TH_ERR_OK;
}

#endif /* TH_CONFIG_WITH_POLL */
