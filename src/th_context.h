#ifndef TH_CONTEXT_H
#define TH_CONTEXT_H

#include "th_allocator.h"
#include "th_config.h"
#include "th_io_composite.h"
#include "th_io_service.h"
#include "th_runner.h"

typedef struct th_context {
    th_runner runner;
    th_allocator* allocator;
    th_io_service* io_service;
} th_context;

TH_PRIVATE(th_err)
th_context_init(th_context* context, th_allocator* allocator);

TH_PRIVATE(th_err)
th_context_init_with_service(th_context* context, th_io_service* service) TH_MAYBE_UNUSED;

TH_PRIVATE(void)
th_context_push_task(th_context* context, th_task* task) TH_MAYBE_UNUSED;

TH_PRIVATE(th_err)
th_context_create_handle(th_context* context, th_io_handle** out, int fd);

TH_PRIVATE(th_err)
th_context_poll(th_context* context, int timeout_ms);

TH_PRIVATE(void)
th_context_drain(th_context* context);

TH_PRIVATE(void)
th_context_deinit(th_context* context);

TH_PRIVATE(void)
th_context_dispatch_handler(th_context* context, th_io_handler* handler, size_t result, th_err err);

TH_PRIVATE(void)
th_context_dispatch_composite_completion(th_context* context, th_io_composite* composite, size_t result, th_err err) TH_MAYBE_UNUSED;

#endif
