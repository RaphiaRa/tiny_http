#include "th_exchange.h"

#include "th_fmt.h"
#include "th_http_error.h"
#include "th_log.h"
#include "th_request.h"
#include "th_request_parser.h"
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
    th_buf_vec buf;
    th_request_parser parser;
    th_request request;
    th_response response;
    th_request_read_mode mode;
    enum {
        TH_EXCHANGE_STATE_READ,
        TH_EXCHANGE_STATE_WRITE,
    } state;
    size_t read;
    size_t parsed;
    bool close;
};

TH_LOCAL(void)
th_exchange_destroy(void* self)
{
    th_exchange* handler = self;
    TH_LOG_TRACE("%p: Destroying", handler);
    th_request_deinit(&handler->request);
    th_response_deinit(&handler->response);
    th_buf_vec_deinit(&handler->buf);
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
    handler->state = TH_EXCHANGE_STATE_WRITE;
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
    handler->state = TH_EXCHANGE_STATE_WRITE;
    th_response_async_write(&handler->response, handler->socket, (th_io_handler*)handler);
}

TH_LOCAL(th_err)
th_exchange_handle_options(th_exchange* exchange, th_request* request, th_response* response)
{
    // All the methods we gotta check
    static const struct {
        th_method method;
        const char* allow;
    } methods[] = {
        {TH_METHOD_GET, "GET, HEAD"},
        {TH_METHOD_POST, "POST"},
        {TH_METHOD_PUT, "PUT"},
        {TH_METHOD_DELETE, "DELETE"},
        {TH_METHOD_PATCH, "PATCH"},
    };
    char allow[512] = {0};
    size_t pos = th_fmt_str_append(allow, 0, sizeof(allow), "OPTIONS"); // OPTIONS is always allowed
    if (strcmp(th_heap_string_data(&request->uri_path), "*") != 0) {
        th_router* router = exchange->router;
        for (size_t i = 0; i < TH_ARRAY_SIZE(methods); i++) {
            if (th_router_would_handle(router, methods[i].method, request)) {
                pos += th_fmt_str_append(allow, pos, sizeof(allow) - pos, ", ");
                pos += th_fmt_str_append(allow, pos, sizeof(allow) - pos, methods[i].allow);
            }
        }
    } else {
        for (size_t i = 0; i < TH_ARRAY_SIZE(methods); i++) {
            pos += th_fmt_str_append(allow, pos, sizeof(allow) - pos, ", ");
            pos += th_fmt_str_append(allow, pos, sizeof(allow) - pos, methods[i].allow);
        }
    }
    th_err err = TH_ERR_OK;
    if ((err = th_response_add_header(response, TH_STRING("Allow"), th_string_make(allow, pos))) != TH_ERR_OK)
        return err;
    if ((err = th_response_add_header(response, TH_STRING("Content-Type"), TH_STRING("text/plain"))) != TH_ERR_OK)
        return err;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_exchange_handle_route(th_exchange* exchange, th_request* request, th_response* response)
{
    th_router* router = exchange->router;
    if (request->method == TH_METHOD_OPTIONS) {
        return th_exchange_handle_options(exchange, request, response);
    } else {
        return th_http_error(th_router_handle(router, request, response));
    }
}

TH_LOCAL(void)
th_exchange_handle_request(th_exchange* handler)
{
    th_socket* socket = handler->socket;
    th_request* request = &handler->request;
    th_response* response = &handler->response;
    // We only need to write headers if it's a HEAD request
    response->only_headers = (request->method == TH_METHOD_HEAD);
    th_err err = th_http_error(th_exchange_handle_route(handler, request, response));
    switch (th_http_code_get_type(TH_ERR_CODE(err))) {
    case TH_HTTP_CODE_TYPE_INFORMATIONAL:
        if (request->version == 0) {
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
    handler->state = TH_EXCHANGE_STATE_WRITE;
    th_response_async_write(response, socket, (th_io_handler*)handler);
}

TH_LOCAL(void)
th_exchange_handle_read(th_exchange* handler, size_t len, th_err err)
{
    if (err != TH_ERR_OK) {
        th_io_composite_complete(&handler->base, TH_EXCHANGE_CLOSE, err);
        return;
    }
    handler = (th_exchange*)th_io_composite_ref(&handler->base);
    handler->read += len;
    size_t parsed = 0;
    th_string parser_input = (th_string){.ptr = th_buf_vec_at(&handler->buf, handler->parsed),
                                         .len = handler->read - handler->parsed};
    if ((err = th_request_parser_parse(&handler->parser, &handler->request, parser_input, &parsed)) != TH_ERR_OK) {
        th_exchange_write_error_response(handler, th_http_error(err));
        return;
    }
    if (th_request_parser_done(&handler->parser)) {
        th_exchange_handle_request(handler);
        return;
    }
    // If we haven't parsed the whole request, we need to read more data
    handler->parsed += parsed;
    if (!th_request_parser_header_done(&handler->parser)) {
        if (handler->read == th_buf_vec_size(&handler->buf)) {
            if (th_buf_vec_size(&handler->buf) < TH_CONFIG_LARGE_HEADER_LEN) {
                th_buf_vec_resize(&handler->buf, TH_CONFIG_LARGE_HEADER_LEN);
            } else {
                th_exchange_write_error_response(handler, TH_ERR_HTTP(TH_CODE_REQUEST_HEADER_FIELDS_TOO_LARGE));
                return;
            }
        }
        th_socket_async_read(handler->socket, th_buf_vec_at(&handler->buf, handler->read),
                             th_buf_vec_size(&handler->buf) - handler->read, (th_io_handler*)handler);
    } else {
        if (handler->mode == TH_REQUEST_READ_MODE_REJECT_UNAVAILABLE) {
            th_exchange_write_error_response(handler, TH_ERR_HTTP(TH_CODE_SERVICE_UNAVAILABLE));
            return;
        } else if (handler->mode == TH_REQUEST_READ_MODE_REJECT_TOO_MANY_REQUESTS) {
            th_exchange_write_error_response(handler, TH_ERR_HTTP(TH_CODE_TOO_MANY_REQUESTS));
            return;
        }
        size_t content_received = handler->read - handler->parsed;
        size_t remaining = th_request_parser_content_len(&handler->parser) - content_received;
        if (handler->read + remaining > th_buf_vec_size(&handler->buf)) {
            memcpy(th_buf_vec_at(&handler->buf, 0), th_buf_vec_at(&handler->buf, handler->parsed), content_received);
            handler->read = content_received;
            if (th_request_parser_content_len(&handler->parser) > th_buf_vec_size(&handler->buf)) {
                th_buf_vec_resize(&handler->buf, th_request_parser_content_len(&handler->parser));
            }
        }
        th_socket_async_read_exact(handler->socket, th_buf_vec_at(&handler->buf, handler->read),
                                   remaining, (th_io_handler*)handler);
    }
}

TH_LOCAL(void)
th_exchange_fn(void* self, size_t len, th_err err)
{
    (void)len;
    th_exchange* handler = self;
    switch (handler->state) {
    case TH_EXCHANGE_STATE_READ: {
        th_exchange_handle_read(handler, len, err);
        return;
    }
    case TH_EXCHANGE_STATE_WRITE: {
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
    th_buf_vec_init(&exchange->buf, exchange->allocator);
    th_request_parser_init(&exchange->parser);
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
    handler->mode = mode;
    handler->state = TH_EXCHANGE_STATE_READ;
    TH_LOG_TRACE("%p: Reading request %p", handler, &handler->request);
    th_buf_vec_resize(&handler->buf, TH_CONFIG_SMALL_HEADER_LEN);
    th_socket_async_read(handler->socket, th_buf_vec_begin(&handler->buf), th_buf_vec_size(&handler->buf), (th_io_handler*)handler);
}
