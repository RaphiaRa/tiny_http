#include "th_exchange.h"

#include "th_http_error.h"
#include "th_log.h"
#include "th_request.h"
#include "th_response.h"

#include <th.h>

#undef TH_LOG_TAG
#define TH_LOG_TAG "exchange"

/**
 * @brief th_exchange receives a http messages, processes it and sends a response.
 */
struct th_exchange {
    th_io_composite base;
    th_allocator* allocator;
    th_socket* socket;
    th_router* router;
    th_fcache* fcache;
    th_request request;
    th_response response;
    enum {
        TH_EXCHANGE_STATE_START,
        TH_EXCHANGE_STATE_HANDLE,
    } state;
    bool close;
};

TH_LOCAL(void)
th_exchange_destroy(void* self)
{
    th_exchange* handler = self;
    TH_LOG_TRACE("%p: Destroying", handler);
    th_request_deinit(&handler->request);
    th_response_deinit(&handler->response);
    th_allocator_free(handler->allocator, handler);
}

TH_LOCAL(void)
th_exchange_write_error_response(th_exchange* handler, th_err err)
{
    th_err http_error = th_http_error(err);
    th_response_set_code(&handler->response, TH_ERR_CODE(http_error));
    th_printf_body(&handler->response, "%d %s", TH_ERR_CODE(http_error), th_http_strerror(http_error));
    th_response_add_header(&handler->response, TH_STRING("Connection"), TH_STRING("close"));
    handler->close = true;
    handler->state = TH_EXCHANGE_STATE_HANDLE;
    th_response_async_write(&handler->response, handler->socket, (th_io_handler*)handler);
}

TH_LOCAL(void)
th_exchange_write_require_1_1_response(th_exchange* handler)
{
    TH_LOG_ERROR("%p: Trying send a HTTP/1.1 response to a HTTP/1.0 client, sending 400 Bad Request instead", handler);
    th_response_set_code(&handler->response, TH_CODE_BAD_REQUEST);
    th_response_set_body(&handler->response, TH_STRING("HTTP/1.1 required for this request"));
    th_response_add_header(&handler->response, TH_STRING("Connection"), TH_STRING("close"));
    handler->close = true;
    handler->state = TH_EXCHANGE_STATE_HANDLE;
    th_response_async_write(&handler->response, handler->socket, (th_io_handler*)handler);
}

TH_LOCAL(void)
th_exchange_handle_request(th_exchange* handler)
{
    handler = (th_exchange*)th_io_composite_ref(&handler->base);
    th_socket* socket = handler->socket;
    th_request* request = &handler->request;
    th_router* router = handler->router;
    th_response* response = &handler->response;
    th_err err = th_http_error(th_router_handle(router, request, response));
    switch (th_http_code_get_type(TH_ERR_CODE(err))) {
    case TH_HTTP_CODE_TYPE_INFORMATIONAL:
        if (request->minor_version == 0) {
            th_exchange_write_require_1_1_response(handler);
            return;
        }
        break;
    case TH_HTTP_CODE_TYPE_ERROR:
        th_exchange_write_error_response(handler, err);
        return;
    default:
        // All other types don't require any special handling
        break;
    }
    if (request->close) {
        th_response_add_header(response, TH_STRING("Connection"), TH_STRING("close"));
        handler->close = true;
    } else {
        th_response_add_header(response, TH_STRING("Connection"), TH_STRING("keep-alive"));
    }
    TH_LOG_TRACE("%p: Write response %p", handler, response);
    handler->state = TH_EXCHANGE_STATE_HANDLE;
    th_response_async_write(response, socket, (th_io_handler*)handler);
}

TH_LOCAL(bool)
th_exchange_is_io_error(th_err err)
{
    return err == TH_ERR_EOF || err == TH_EBADF || err == TH_EIO;
}

TH_LOCAL(void)
th_exchange_fn(void* self, size_t len, th_err err)
{
    (void)len;
    th_exchange* handler = self;
    switch (handler->state) {
    case TH_EXCHANGE_STATE_START: {
        if (err != TH_ERR_OK) {
            if (!th_exchange_is_io_error(err)) {
                // Unless it's an I/O error, we should send a response
                TH_LOG_DEBUG("%p: Rejecting request with error %s", handler, th_strerror(err));
                th_exchange_write_error_response((th_exchange*)th_io_composite_ref(&handler->base), err);
            } else {
                TH_LOG_DEBUG("%p: Failed to read request: %s", handler, th_strerror(err));
                th_io_composite_complete(&handler->base, TH_EXCHANGE_CLOSE, err);
            }
            return;
        }
        TH_LOG_TRACE("%p: Read request %p of length %zu", handler, &handler->request, len);
        th_exchange_handle_request(handler);
        break;
    }
    case TH_EXCHANGE_STATE_HANDLE: {
        if (err != TH_ERR_OK) {
            TH_LOG_ERROR("%p: Failed to write response: %s", handler, th_strerror(err));
            th_io_composite_complete(&handler->base, TH_EXCHANGE_CLOSE, err);
            return;
        }
        TH_LOG_TRACE("%p: Wrote response %p of length %zu", handler, &handler->response, len);
        size_t result = handler->close ? TH_EXCHANGE_CLOSE : TH_EXCHANGE_CONTINUE;
        th_io_composite_complete(&handler->base, result, TH_ERR_OK);
        break;
    }
    }
}

TH_LOCAL(void)
th_exchange_init(th_exchange* exchange, th_socket* socket,
                 th_router* router, th_fcache* fcache,
                 th_allocator* allocator, th_io_handler* on_complete)
{
    th_io_composite_init(&exchange->base, th_exchange_fn, th_exchange_destroy, on_complete);
    exchange->socket = socket;
    exchange->router = router;
    exchange->fcache = fcache;
    exchange->allocator = allocator ? allocator : th_default_allocator_get();
    th_request_init(&exchange->request, allocator);
    th_response_init(&exchange->response, exchange->fcache, allocator);
    exchange->close = false;
}

TH_PRIVATE(th_err)
th_exchange_create(th_exchange** out, th_socket* socket,
                   th_router* router, th_fcache* fcache,
                   th_allocator* allocator, th_io_handler* on_complete)
{
    th_exchange* handler = th_allocator_alloc(allocator, sizeof(th_exchange));
    if (!handler) {
        return TH_ERR_BAD_ALLOC;
    }
    th_exchange_init(handler, socket, router, fcache, allocator, on_complete);
    *out = handler;
    return TH_ERR_OK;
}

TH_PRIVATE(void)
th_exchange_start(th_exchange* handler, th_request_read_mode mode)
{
    handler->state = TH_EXCHANGE_STATE_START;
    TH_LOG_TRACE("%p: Reading request %p", handler, &handler->request);
    th_request_async_read(handler->socket, handler->allocator, &handler->request, mode, (th_io_handler*)handler);
}
