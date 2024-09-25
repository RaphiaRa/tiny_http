#ifndef TH_IO_SERVICE_H
#define TH_IO_SERVICE_H

#include <th.h>

#include "th_io_task.h"
#include "th_task.h"
#include "th_utility.h"

typedef struct th_io_handle {
    void (*cancel)(void* self);
    void (*submit)(void* self, th_io_task* task);
    void (*enable_timeout)(void* self, bool enabled);
    int (*get_fd)(void* self);
    void (*destroy)(void* self);
} th_io_handle;

TH_INLINE(void)
th_io_handle_cancel(th_io_handle* io_handle)
{
    io_handle->cancel(io_handle);
}

TH_INLINE(void)
th_io_handle_submit(th_io_handle* io_handle, th_io_task* iot)
{
    io_handle->submit(io_handle, iot);
}

TH_INLINE(int)
th_io_handle_get_fd(th_io_handle* io_handle)
{
    return io_handle->get_fd(io_handle);
}

TH_INLINE(void)
th_io_handle_enable_timeout(th_io_handle* io_handle, bool enabled)
{
    io_handle->enable_timeout(io_handle, enabled);
}

TH_INLINE(void)
th_io_handle_destroy(th_io_handle* io_handle)
{
    io_handle->destroy(io_handle);
}

typedef struct th_io_service {
    void (*run)(void* self, int timeout_ms);
    th_err (*create_handle)(void* self, th_io_handle** out, int fd);
    void (*destroy)(void* self);
} th_io_service;

TH_INLINE(void)
th_io_service_run(th_io_service* io_service, int timeout_ms)
{
    io_service->run(io_service, timeout_ms);
}

TH_INLINE(th_err)
th_io_service_create_handle(th_io_service* io_service, th_io_handle** out, int fd)
{
    return io_service->create_handle(io_service, out, fd);
}

TH_INLINE(void)
th_io_service_destroy(th_io_service* io_service)
{
    if (io_service->destroy)
        io_service->destroy(io_service);
}

#endif
