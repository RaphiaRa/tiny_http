#include "th_context.h"
#include "th_io_service.h"
#include "th_kqueue_service.h"
#include "th_log.h"
#include "th_mock_service.h"
#include "th_poll_service.h"

#undef TH_LOG_TAG
#define TH_LOG_TAG "context"

TH_LOCAL(th_err)
th_io_service_create(th_io_service** out, th_runner* runner, th_allocator* allocator)
{
    allocator = allocator ? allocator : th_default_allocator_get();
    (void)out;
#if defined(TH_CONFIG_OS_MOCK)
    (void)allocator;
    TH_LOG_INFO("Using mock");
    return th_mock_service_create(out, runner);
#endif
#if defined(TH_CONFIG_WITH_KQUEUE)
    TH_LOG_INFO("Using kqueue");
    return th_kqueue_service_create(out, runner, allocator);
#endif
#if defined(TH_CONFIG_WITH_POLL)
    TH_LOG_INFO("Using poll");
    return th_poll_service_create(out, runner, allocator);
#endif
    TH_LOG_ERROR("No IO service implementation available");
    return TH_ERR_NOSUPPORT;
}

TH_PRIVATE(th_err)
th_context_init(th_context* context, th_allocator* allocator)
{
    th_err err = TH_ERR_OK;
    context->allocator = allocator ? allocator : th_default_allocator_get();
    th_runner_init(&context->runner);
    if ((th_io_service_create(&context->io_service, &context->runner, context->allocator)) != TH_ERR_OK) {
        return err;
    }
    th_runner_set_io_service(&context->runner, context->io_service);
    return TH_ERR_OK;
}

TH_PRIVATE(th_err)
th_context_init_with_service(th_context* context, th_io_service* service)
{
    context->io_service = service;
    th_runner_init(&context->runner);
    th_runner_set_io_service(&context->runner, context->io_service);
    return TH_ERR_OK;
}

TH_PRIVATE(void)
th_context_push_task(th_context* context, th_task* task)
{
    th_runner_push_task(&context->runner, task);
}

TH_PRIVATE(th_err)
th_context_create_handle(th_context* context, th_io_handle** out, int fd)
{
    return th_io_service_create_handle(context->io_service, out, fd);
}

TH_PRIVATE(th_err)
th_context_poll(th_context* context, int timeout_ms)
{
    return th_runner_poll(&context->runner, timeout_ms);
}

TH_PRIVATE(void)
th_context_drain(th_context* context)
{
    th_runner_drain(&context->runner);
}

TH_PRIVATE(void)
th_context_deinit(th_context* context)
{
    th_runner_deinit(&context->runner);
    th_io_service_destroy(context->io_service);
}

TH_PRIVATE(void)
th_context_dispatch_handler(th_context* context, th_io_handler* handler, size_t result, th_err err)
{
    th_io_handler_set_result(handler, result, err);
    th_context_push_task(context, &handler->base);
}

TH_PRIVATE(void)
th_context_dispatch_composite_completion(th_context* context, th_io_composite* composite, size_t result, th_err err)
{
    th_context_dispatch_handler(context, TH_MOVE_PTR(composite->on_complete), result, err);
}
