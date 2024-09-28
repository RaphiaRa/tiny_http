#include "th_kqueue_service.h"

#ifdef TH_CONFIG_WITH_KQUEUE
#include "th_log.h"
#include "th_system_error.h"
#include "th_utility.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#undef TH_LOG_TAG
#define TH_LOG_TAG "kqueue_service"

/* th_kqueue_handle forward declarations begin */

TH_LOCAL(void)
th_kqueue_handle_init(th_kqueue_handle* handle, th_kqueue_service* service, int fd, th_allocator* allocator);

TH_LOCAL(void)
th_kqueue_handle_do_cancel(th_kqueue_handle* handle, th_err reason);

/* th_kqueue_handle forward declarations end */
/* th_kqueue_task_dispatcher implementation begin */

TH_LOCAL(const char*)
th_kqueue_fitler_to_string(int filter) TH_MAYBE_UNUSED;

TH_LOCAL(const char*)
th_kqueue_flags_to_string(int flags) TH_MAYBE_UNUSED;

TH_LOCAL(const char*)
th_kqueue_fitler_to_string(int filter)
{
    switch (filter) {
    case EVFILT_READ:
        return "EVFILT_READ";
    case EVFILT_WRITE:
        return "EVFILT_WRITE";
    case EVFILT_TIMER:
        return "EVFILT_TIMER";
    default:
        return "UNKNOWN";
    }
}

TH_LOCAL(const char*)
th_kqueue_flags_to_string(int flags)
{
    static char buf[256];
    buf[0] = '[';
    buf[1] = '\0';
    if (flags & EV_ADD)
        strncat(buf, "EV_ADD ", sizeof(buf) - strlen(buf) - 1);
    if (flags & EV_DELETE)

        strncat(buf, "EV_DELETE ", sizeof(buf) - strlen(buf) - 1);
    if (flags & EV_ENABLE)
        strncat(buf, "EV_ENABLE ", sizeof(buf) - strlen(buf) - 1);
    if (flags & EV_DISABLE)
        strncat(buf, "EV_DISABLE ", sizeof(buf) - strlen(buf) - 1);
    if (flags & EV_ONESHOT)
        strncat(buf, "EV_ONESHOT ", sizeof(buf) - strlen(buf) - 1);
    if (flags & EV_CLEAR)
        strncat(buf, "EV_CLEAR ", sizeof(buf) - strlen(buf) - 1);
    if (flags & EV_EOF)
        strncat(buf, "EV_EOF ", sizeof(buf) - strlen(buf) - 1);
    if (flags & EV_ERROR)
        strncat(buf, "EV_ERROR ", sizeof(buf) - strlen(buf) - 1);
    if (flags & EV_OOBAND)
        strncat(buf, "EV_OOBAND ", sizeof(buf) - strlen(buf) - 1);
    strncat(buf, "]", sizeof(buf) - strlen(buf) - 1);
    return buf;
}

TH_LOCAL(void)
th_kqueue_service_run(void* self, int timeout_ms)
{
    th_kqueue_service* service = self;

    static const int max_events = 128;
    struct kevent evlist[max_events] = {0};

    struct timespec timeout = {
        .tv_sec = timeout_ms / 1000,
        .tv_nsec = (timeout_ms % 1000) * 1000000,
    };

    int nev = kevent(service->kq, NULL, 0, evlist, max_events, timeout_ms == -1 ? NULL : &timeout);
    if (nev == -1) {
        TH_LOG_ERROR("kevent failed: %s", strerror(errno));
        return;
    }
    for (int i = 0; i < nev; ++i) {
        TH_LOG_TRACE("kevent: fd=%d, filter=%s, flags=%s, data=%d",
                     (int)evlist[i].ident, th_kqueue_fitler_to_string(evlist[i].filter),
                     th_kqueue_flags_to_string(evlist[i].flags), (int)evlist[i].data);

        th_kqueue_handle* handle = evlist[i].udata;
        th_io_op_type op_type = TH_IO_OP_TYPE_NONE;
        switch (evlist[i].filter) {
        case EVFILT_READ:
            op_type = TH_IO_OP_TYPE_READ;
            break;
        case EVFILT_WRITE:
            op_type = TH_IO_OP_TYPE_WRITE;
            break;
        default:
            TH_ASSERT(0 && "Invalid filter");
            break;
        }
        int idx = op_type - 1;
        if (handle->iot[idx]) {
            if (evlist[i].flags & EV_ERROR) {
                th_runner_push_uncounted_task(service->runner, (th_task*)th_io_task_abort(TH_MOVE_PTR(handle->iot[idx]), TH_ERR_SYSTEM(errno)));
            } else if (evlist[i].flags & EV_EOF && evlist[i].data == 0) {
                th_runner_push_uncounted_task(service->runner, (th_task*)th_io_task_abort(TH_MOVE_PTR(handle->iot[idx]), TH_ERR_EOF));
            } else {
                th_runner_push_uncounted_task(service->runner, (th_task*)TH_MOVE_PTR(handle->iot[idx]));
            }
            if (handle->timeout_enabled)
                th_kqueue_timer_list_erase(&service->timer_list, handle);
        }
    }
    th_kqueue_handle* handle = NULL;
    while ((handle = th_kqueue_timer_list_front(&service->timer_list)) != NULL) {
        if (!th_timer_expired(&handle->timer))
            break;
        (void)th_kqueue_timer_list_pop_front(&service->timer_list);
        th_kqueue_handle_do_cancel(handle, TH_ERR_SYSTEM(TH_ETIMEDOUT));
    }
}

TH_LOCAL(void)
th_kqueue_service_deinit(th_kqueue_service* service)
{
    th_kqueue_handle_pool_deinit(&service->handle_allocator);
    close(service->kq);
}

TH_LOCAL(void)
th_kqueue_service_destroy(void* self)
{
    th_kqueue_service* service = self;
    th_kqueue_service_deinit(service);
    th_allocator_free(service->allocator, service);
}

TH_LOCAL(th_err)
th_kqueue_service_create_handle(void* self, th_io_handle** out, int fd)
{
    th_kqueue_service* service = self;
    th_kqueue_handle* handle = th_allocator_alloc(&service->handle_allocator.base, sizeof(th_kqueue_handle));
    if (!handle)
        return TH_ERR_SYSTEM(errno);
    th_kqueue_handle_init(handle, service, fd, &service->handle_allocator.base);
    *out = (th_io_handle*)handle;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_kqueue_service_init(th_kqueue_service* service, th_runner* runner, th_allocator* allocator)
{
    service->base.create_handle = th_kqueue_service_create_handle;
    service->base.run = th_kqueue_service_run;
    service->base.destroy = th_kqueue_service_destroy;
    service->allocator = allocator;
    service->runner = runner;
    if ((service->kq = kqueue()) == -1) {
        return TH_ERR_SYSTEM(errno);
    }
    th_kqueue_handle_pool_init(&service->handle_allocator, service->allocator, 16, TH_CONFIG_MAX_HANDLES);
    return TH_ERR_OK;
}

TH_PRIVATE(th_err)
th_kqueue_service_create(th_io_service** out, th_runner* runner, th_allocator* allocator)
{
    allocator = allocator ? allocator : th_default_allocator_get();
    th_kqueue_service* service = th_allocator_alloc(allocator, sizeof(th_kqueue_service));
    if (!service)
        return TH_ERR_SYSTEM(errno);
    th_err err = th_kqueue_service_init(service, runner, allocator);
    if (err != TH_ERR_OK) {
        th_allocator_free(allocator, service);
        return err;
    }
    *out = (th_io_service*)service;
    return TH_ERR_OK;
}

/* th_kqueue_task_dispatcher implementation end */
/* th_kqueue_handle implementation begin */

TH_LOCAL(void)
th_kqueue_handle_do_cancel(th_kqueue_handle* handle, th_err reason)
{
    th_io_task* iot[TH_IO_OP_TYPE_MAX] = {0};
    size_t count = 0;
    for (int i = 0; i < TH_IO_OP_TYPE_MAX; ++i) {
        if (handle->iot[i]) {
            iot[count++] = TH_MOVE_PTR(handle->iot[i]);
        }
    }
    for (size_t i = 0; i < count; ++i) {
        th_runner_push_uncounted_task(handle->service->runner, (th_task*)th_io_task_abort(iot[i], reason));
    }
}
TH_LOCAL(void)
th_kqueue_handle_cancel(void* self)
{
    th_kqueue_handle* handle = self;
    th_kqueue_handle_do_cancel(handle, TH_ERR_SYSTEM(TH_ECANCELED));
}

TH_LOCAL(void)
th_kqueue_handle_submit(void* self, th_io_task* task)
{
    th_kqueue_handle* handle = self;
    th_io_op_type op_type = TH_IO_OP_TYPE(task->op);
    th_io_handler* on_complete = th_io_task_try_execute(task);
    if (on_complete) {
        th_runner_push_task(handle->service->runner, (th_task*)on_complete);
        return;
    }

    if ((handle->active & op_type) == 0) {

        struct kevent ev = {0};
        switch (op_type) {
        case TH_IO_OP_TYPE_READ:
            EV_SET(&ev, handle->fd, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, handle);
            break;
        case TH_IO_OP_TYPE_WRITE:
            EV_SET(&ev, handle->fd, EVFILT_WRITE, EV_ADD | EV_CLEAR, 0, 0, handle);
            break;
        default:
            TH_ASSERT(0 && "Invalid op type");
            break;
        }
        if (kevent(handle->service->kq, &ev, 1, NULL, 0, NULL) == -1) {
            th_runner_push_task(handle->service->runner, (th_task*)th_io_task_abort(task, TH_ERR_SYSTEM(errno)));
            return;
        }
        handle->active |= op_type;
    }
    if (handle->timeout_enabled) {
        th_err err = th_timer_set(&handle->timer, th_seconds(TH_CONFIG_IO_TIMEOUT));
        if (err != TH_ERR_OK) {
            TH_LOG_ERROR("Failed to set timer: %s, disabling timeout", th_strerror(err));
            handle->timeout_enabled = false;
        } else {
            th_kqueue_timer_list_push_back(&handle->service->timer_list, handle);
        }
    }

    th_runner_increase_task_count(handle->service->runner);
    handle->iot[op_type - 1] = task;
}

TH_LOCAL(int)
th_kqueue_handle_get_fd(void* self)
{
    th_kqueue_handle* handle = self;
    return handle->fd;
}

TH_LOCAL(void)
th_kqueue_handle_enable_timeout(void* self, bool enabled)
{
    th_kqueue_handle* handle = self;
    handle->timeout_enabled = enabled;
}

TH_LOCAL(void)
th_kqueue_handle_destroy(void* self)
{
    th_kqueue_handle* handle = self;
    th_kqueue_handle_cancel(handle);
    close(handle->fd);
    th_allocator_free(handle->allocator, handle);
}

TH_LOCAL(void)
th_kqueue_handle_init(th_kqueue_handle* handle, th_kqueue_service* service, int fd, th_allocator* allocator)
{
    handle->base.cancel = th_kqueue_handle_cancel;
    handle->base.submit = th_kqueue_handle_submit;
    handle->base.enable_timeout = th_kqueue_handle_enable_timeout;
    handle->base.get_fd = th_kqueue_handle_get_fd;
    handle->base.destroy = th_kqueue_handle_destroy;
    handle->allocator = allocator;
    handle->iot[TH_IO_OP_TYPE_READ - 1] = NULL;
    handle->iot[TH_IO_OP_TYPE_WRITE - 1] = NULL;
    handle->service = service;
    handle->fd = fd;
    handle->active = TH_IO_OP_TYPE_NONE;
    th_timer_init(&handle->timer);
}

/* th_kqueue_handle implementation end */

#endif /* TH_HAVE_KQUEUE */
