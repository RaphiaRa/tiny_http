#include "th_http.h"
#include "th_fmt.h"
#include "th_http_error.h"

#include <stdbool.h>

#undef TH_LOG_TAG
#define TH_LOG_TAG "http"

#define TH_HTTP_CLOSE true
#define TH_HTTP_KEEP_ALIVE false

TH_LOCAL(void)
th_http_destroy(void* self)
{
    th_http* http = self;
    TH_LOG_TRACE("%p: Destroying http protocol instance", http);
    th_conn_destroy(http->conn);
    th_request_deinit(&http->request);
    th_response_deinit(&http->response);
    th_buf_vec_deinit(&http->buf);
    th_allocator_free(http->allocator, http);
}

TH_LOCAL(void)
th_http_restart(th_http* http)
{
    http->read_bytes = 0;
    http->parsed_bytes = 0;
    th_request_parser_reset(&http->parser);
    th_request_reset(&http->request);
    th_response_reset(&http->response);
    http->state = TH_HTTP_STATE_READ_REQUEST;
    th_socket_async_read(th_conn_get_socket(http->conn), th_buf_vec_at(&http->buf, 0), th_buf_vec_size(&http->buf), &http->io_handler.base);
}

TH_LOCAL(void)
th_http_complete(th_http* http)
{
    if (http->close) {
        th_http_destroy(http);
    } else {
        th_http_restart(http);
    }
}

TH_LOCAL(void)
th_http_write_response(th_http* http)
{
    http->state = TH_HTTP_STATE_WRITE_RESPONSE;
    th_response_async_write(&http->response, th_conn_get_socket(http->conn), &http->io_handler.base);
}

TH_LOCAL(void)
th_http_write_error_response(th_http* http, th_err err)
{
    th_response_set_code(&http->response, TH_ERR_CODE(err));
    if (th_heap_string_len(&http->request.uri_path) == 0) {
        // Set default error message
        th_printf_body(&http->response, "%d %s", TH_ERR_CODE(err), th_http_strerror(err));
    }
    if (http->close) {
        th_response_add_header(&http->response, TH_STRING("Connection"), TH_STRING("close"));
        http->close = TH_HTTP_CLOSE;
    }
    th_http_write_response(http);
}

TH_LOCAL(void)
th_http_handle_error(th_http* http, th_err err)
{
    th_http_code_type type = th_http_code_get_type(TH_ERR_CODE(err));
    switch (type) {
    case TH_HTTP_CODE_TYPE_SERVER_ERROR:
        http->close = TH_HTTP_CLOSE;
        break;
    case TH_HTTP_CODE_TYPE_CLIENT_ERROR:
        break;
    default:
        TH_ASSERT(0 && "Invalid error type");
        break;
    }
    th_http_write_error_response(http, err);
}

TH_LOCAL(void)
th_http_handle_require_1_1(th_http* http)
{
    TH_LOG_ERROR("%p: Trying send a HTTP/1.1 response to a HTTP/1.0 client, sending 400 Bad Request instead", http);
    th_response_set_body(&http->response, TH_STRING("HTTP/1.1 required for this request"));
    th_http_handle_error(http, TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
}

TH_LOCAL(th_err)
th_http_handle_options(th_router* router, th_request* request, th_response* response)
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
th_http_handle_route(th_router* router, th_request* request, th_response* response)
{
    if (request->method == TH_METHOD_OPTIONS) {
        return th_http_handle_options(router, request, response);
    } else {
        return th_router_handle(router, request, response);
    }
}

TH_LOCAL(void)
th_http_prehandle_request(th_http* http)
{
    th_request* request = &http->request;
    th_response* response = &http->response;
    if (request->method == TH_METHOD_HEAD) {
        response->only_headers = true;   // only write headers
        request->method = TH_METHOD_GET; // pretend it's a GET request
    }
}

TH_LOCAL(void)
th_http_handle_request_and_write_response(th_http* http)
{
    th_request* request = &http->request;
    th_response* response = &http->response;
    th_http_prehandle_request(http);
    th_err err = th_http_error(th_http_handle_route(http->router, &http->request, &http->response));
    switch (th_http_code_get_type(TH_ERR_CODE(err))) {
    case TH_HTTP_CODE_TYPE_INFORMATIONAL:
        if (request->version == 0) {
            th_http_handle_require_1_1(http);
            return;
        }
        break;
    case TH_HTTP_CODE_TYPE_SERVER_ERROR:
    case TH_HTTP_CODE_TYPE_CLIENT_ERROR:
        th_http_handle_error(http, err);
        return;
    default:
        // All other types don't require any special handling
        break;
    }
    // All good, write the response
    if (request->close) {
        th_response_add_header(response, TH_STRING("Connection"), TH_STRING("close"));
        http->close = true;
    } else {
        th_response_add_header(response, TH_STRING("Connection"), TH_STRING("keep-alive"));
    }
    TH_LOG_TRACE("%p: Write response %p", http, response);
    th_http_write_response(http);
}

TH_LOCAL(void)
th_http_handle_read_request(th_http* http, size_t len, th_err err)
{
    if (err != TH_ERR_OK) {
        TH_LOG_DEBUG("%p: Read error: %s", http, th_strerror(err));
        http->close = TH_HTTP_CLOSE; // No other choice if we can't even read the request
        th_http_complete(http);
        return;
    }
    http->read_bytes += len;
    size_t parsed = 0;
    th_string parser_input = (th_string){.ptr = th_buf_vec_at(&http->buf, http->parsed_bytes),
                                         .len = http->read_bytes - http->parsed_bytes};
    if ((err = th_request_parser_parse(&http->parser, &http->request, parser_input, &parsed)) != TH_ERR_OK) {
        th_http_write_error_response(http, th_http_error(err));
        return;
    }
    if (th_request_parser_done(&http->parser)) {
        th_http_handle_request_and_write_response(http);
        return;
    }
    // If we haven't parsed the whole request, we need to read more data
    http->parsed_bytes += parsed;
    if (!th_request_parser_header_done(&http->parser)) {
        if (http->read_bytes == th_buf_vec_size(&http->buf)) {
            if (th_buf_vec_size(&http->buf) < TH_CONFIG_LARGE_HEADER_LEN) {
                th_buf_vec_resize(&http->buf, TH_CONFIG_LARGE_HEADER_LEN);
            } else {
                th_http_write_error_response(http, TH_ERR_HTTP(TH_CODE_REQUEST_HEADER_FIELDS_TOO_LARGE));
                return;
            }
        }
        th_socket_async_read(th_conn_get_socket(http->conn), th_buf_vec_at(&http->buf, http->read_bytes),
                             th_buf_vec_size(&http->buf) - http->read_bytes, &http->io_handler.base);
    } else {
        if (th_conn_tracker_count(http->tracker) > TH_CONFIG_MAX_CONNECTIONS) {
            TH_LOG_WARN("Too many connections, rejecting new connection");
            th_http_write_error_response(http, TH_ERR_HTTP(TH_CODE_SERVICE_UNAVAILABLE));
            return;
        }
        size_t content_received = http->read_bytes - http->parsed_bytes;
        size_t content_len = th_request_parser_content_len(&http->parser);
        if (content_len > TH_MAX_BODY_LEN) {
            TH_LOG_WARN("Request body too large, rejecting request");
            th_http_write_error_response(http, TH_ERR_HTTP(TH_CODE_PAYLOAD_TOO_LARGE));
            return;
        }
        size_t remaining = content_len - content_received;
        if (http->read_bytes + remaining > th_buf_vec_size(&http->buf)) {
            memcpy(th_buf_vec_at(&http->buf, 0), th_buf_vec_at(&http->buf, http->parsed_bytes), content_received);
            http->read_bytes = content_received;
            http->parsed_bytes = 0;
            if (content_len > th_buf_vec_size(&http->buf)) {
                th_buf_vec_resize(&http->buf, content_len);
            }
        }
        th_socket_async_read_exact(th_conn_get_socket(http->conn), th_buf_vec_at(&http->buf, http->read_bytes),
                                   remaining, &http->io_handler.base);
    }
}

TH_LOCAL(void)
th_http_handle_write_response(th_http* http, size_t len, th_err err)
{
    (void)len;
    if (err != TH_ERR_OK) {
        TH_LOG_ERROR("%p: Write error: %s", http, th_strerror(err));
        http->close = TH_HTTP_CLOSE; // Connection is broken, close it
    } else {
        TH_LOG_TRACE("%p: Write response of %d bytes", http, (int)len);
    }
    th_http_complete(http);
}

TH_LOCAL(void)
th_http_io_handler_fn(void* self, size_t len, th_err err)
{
    th_http_io_handler* handler = self;
    th_http* http = handler->http;
    switch (http->state) {
        {
        case TH_HTTP_STATE_READ_REQUEST:
            th_http_handle_read_request(http, len, err);
            break;
        case TH_HTTP_STATE_WRITE_RESPONSE:
            th_http_handle_write_response(http, len, err);
            break;
        default:
            TH_ASSERT(0 && "Invalid state");
            break;
        }
    }
}

TH_LOCAL(void)
th_http_io_handler_init(th_http_io_handler* handler, th_http* http)
{
    th_io_handler_init(&handler->base, th_http_io_handler_fn, NULL);
    handler->http = http;
}

TH_LOCAL(void)
th_http_start(void* self)
{
    th_http* http = self;
    TH_LOG_TRACE("%p: Starting", http);
    th_buf_vec_resize(&http->buf, TH_CONFIG_SMALL_HEADER_LEN);
    th_socket_async_read(th_conn_get_socket(http->conn), th_buf_vec_at(&http->buf, 0), th_buf_vec_size(&http->buf), &http->io_handler.base);
}

TH_PRIVATE(void)
th_http_init(th_http* http, const th_conn_tracker* tracker, th_conn* conn,
             th_router* router, th_fcache* fcache, th_allocator* allocator)
{
    allocator = allocator ? allocator : th_default_allocator_get();
    th_http_io_handler_init(&http->io_handler, http);
    th_request_parser_init(&http->parser);
    th_request_init(&http->request, allocator);
    th_response_init(&http->response, fcache, allocator);
    th_buf_vec_init(&http->buf, allocator);
    http->tracker = tracker;
    http->conn = conn;
    http->router = router;
    http->fcache = fcache;
    http->allocator = allocator;
    http->read_bytes = 0;
    http->parsed_bytes = 0;
    http->state = TH_HTTP_STATE_READ_REQUEST;
    http->close = TH_HTTP_KEEP_ALIVE;
}

TH_PRIVATE(th_err)
th_http_create(th_http** out, const th_conn_tracker* tracker, th_conn* conn,
               th_router* router, th_fcache* fcache, th_allocator* allocator)
{
    th_http* http = th_allocator_alloc(allocator, sizeof(th_http));
    if (!http)
        return TH_ERR_BAD_ALLOC;
    th_http_init(http, tracker, conn, router, fcache, allocator);
    *out = http;
    return TH_ERR_OK;
}

TH_PRIVATE(void)
th_http_upgrader_upgrade(void* self, th_conn* conn)
{
    th_http_upgrader* upgrader = self;
    th_http* http = NULL;
    th_err err = TH_ERR_OK;
    if ((err = th_http_create(&http, upgrader->tracker, conn, upgrader->router, upgrader->fcache, upgrader->allocator)) != TH_ERR_OK) {
        TH_LOG_ERROR("Failed to create http instance: %s", th_strerror(err));
        th_conn_destroy(conn);
        return;
    }
    th_http_start(http);
}

TH_PRIVATE(void)
th_http_upgrader_init(th_http_upgrader* upgrader, const th_conn_tracker* tracker, th_router* router,
                      th_fcache* fcache, th_allocator* allocator)
{
    th_conn_upgrader_init(&upgrader->base, th_http_upgrader_upgrade);
    upgrader->tracker = tracker;
    upgrader->router = router;
    upgrader->fcache = fcache;
    upgrader->allocator = allocator;
}
