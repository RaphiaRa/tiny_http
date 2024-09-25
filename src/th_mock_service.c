#include "th_mock_service.h"

#if defined(TH_CONFIG_OS_MOCK)

#include "th_allocator.h"
#include "th_system_error.h"

#include <errno.h>

TH_LOCAL(void)
th_mock_handle_cancel(void* self)
{
    (void)self;
}

TH_LOCAL(void)
th_mock_handle_submit(void* self, th_io_task* task)
{
    th_mock_handle* handle = self;
    th_io_handler* on_complete = th_io_task_try_execute(task);
    if (!on_complete)
        on_complete = th_io_task_abort(task, TH_ERR_SYSTEM(TH_EAGAIN));
    th_runner_push_task(handle->service->runner, (th_task*)on_complete);
}

TH_LOCAL(void)
th_mock_handle_enable_timeout(void* self, bool enabled)
{
    (void)self;
    (void)enabled;
}

TH_LOCAL(int)
th_mock_handle_get_fd(void* self)
{
    th_mock_handle* handle = self;
    return handle->fd;
}

TH_LOCAL(void)
th_mock_handle_destroy(void* self)
{
    th_mock_handle* handle = self;
    th_allocator_free(th_default_allocator_get(), handle);
}

TH_LOCAL(void)
th_mock_handle_init(th_mock_handle* handle, th_mock_service* service, int fd)
{
    handle->base.cancel = th_mock_handle_cancel;
    handle->base.submit = th_mock_handle_submit;
    handle->base.enable_timeout = th_mock_handle_enable_timeout;
    handle->base.get_fd = th_mock_handle_get_fd;
    handle->base.destroy = th_mock_handle_destroy;
    handle->service = service;
    handle->fd = fd;
}

TH_LOCAL(void)
th_mock_service_deinit(th_mock_service* service)
{
    (void)service;
}

TH_LOCAL(void)
th_mock_service_destroy(void* self)
{
    th_mock_service* service = self;
    th_mock_service_deinit(service);
    th_allocator_free(th_default_allocator_get(), service);
}

TH_LOCAL(th_err)
th_mock_service_create_handle(void* self, th_io_handle** out, int fd)
{
    th_mock_service* service = self;
    th_mock_handle* handle = th_allocator_alloc(th_default_allocator_get(), sizeof(th_mock_handle));
    if (!handle)
        return TH_ERR_BAD_ALLOC;
    th_mock_handle_init(handle, service, fd);
    *out = (th_io_handle*)handle;
    return TH_ERR_OK;
}

TH_LOCAL(void)
th_mock_service_run(void* self, int timeout_ms)
{
    (void)self;
    (void)timeout_ms;
}

TH_LOCAL(th_err)
th_mock_service_init(th_mock_service* service, th_runner* runner)
{
    service->base.create_handle = th_mock_service_create_handle;
    service->base.run = th_mock_service_run;
    service->base.destroy = NULL;
    service->runner = runner;
    return TH_ERR_OK;
}

TH_PRIVATE(th_err)
th_mock_service_create(th_io_service** out, th_runner* runner)
{
    th_mock_service* service = th_allocator_alloc(th_default_allocator_get(), sizeof(th_mock_service));
    if (!service)
        return TH_ERR_BAD_ALLOC;
    th_err err = th_mock_service_init(service, runner);
    if (err != TH_ERR_OK) {
        th_allocator_free(th_default_allocator_get(), service);
        return err;
    }
    service->base.destroy = th_mock_service_destroy;
    *out = (th_io_service*)service;
    return TH_ERR_OK;
}
#endif
