#include "th_request.h"

#include "picohttpparser.h"
#include "th_hashmap.h"
#include "th_header_id.h"
#include "th_io_composite.h"
#include "th_log.h"
#include "th_method.h"
#include "th_string.h"
#include "th_url_decode.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#undef TH_LOG_TAG
#define TH_LOG_TAG "request"

typedef enum th_request_read_state {
    TH_REQUEST_READ_STATE_HEADER,
    TH_REQUEST_READ_STATE_PAYLOAD
} th_request_read_state;

typedef struct th_request_read_handler {
    th_io_composite base;
    th_allocator* allocator;
    th_socket* sock;
    th_request* request;
    th_request_read_state state;
} th_request_read_handler;

TH_LOCAL(void)
th_request_read_handler_fn(void* self, size_t len, th_err err);

TH_LOCAL(void)
th_request_read_handler_destroy(void* self)
{
    th_request_read_handler* handler = self;
    th_allocator_free(handler->allocator, handler);
}

TH_LOCAL(void)
th_request_read_handler_complete(th_request_read_handler* handler, size_t len, th_err err)
{
    th_io_composite_complete(&handler->base, len, err);
}

TH_LOCAL(th_err)
th_request_read_handler_create(th_request_read_handler** handler, th_allocator* allocator, th_socket* sock, th_request* request, th_io_handler* on_complete)
{
    *handler = th_allocator_alloc(allocator, sizeof(th_request_read_handler));
    if (!*handler)
        return TH_ERR_BAD_ALLOC;
    th_io_composite_init(&(*handler)->base, th_request_read_handler_fn, th_request_read_handler_destroy, on_complete);
    (*handler)->allocator = allocator;
    (*handler)->sock = sock;
    (*handler)->request = request;
    (*handler)->state = TH_REQUEST_READ_STATE_HEADER;
    return TH_ERR_OK;
}

/* th_request_read_handler end */
/* th_request implementation begin */

TH_LOCAL(th_err)
th_request_parse_next_query_param(th_string string, size_t* pos, th_string* key, th_string* value)
{
    size_t eq = th_string_find_first(string, *pos, '=');
    if (eq == th_string_npos) {
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    }
    *key = th_string_trim(th_string_substr(string, *pos, eq - *pos));
    *pos = th_string_find_first(string, eq + 1, '&');
    if (*pos != th_string_npos) {
        *value = th_string_trim(th_string_substr(string, eq + 1, *pos - eq - 1));
        (*pos)++;
        return TH_ERR_OK;
    } else {
        *value = th_string_trim(th_string_substr(string, eq + 1, *pos));
        return TH_ERR_OK;
    }
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_request_parse_path_query(th_request* request, th_string path)
{
    size_t pos = 0;
    while (pos != th_string_npos) {
        th_string key;
        th_string value;
        th_err err = th_request_parse_next_query_param(path, &pos, &key, &value);
        if (err != TH_ERR_OK) {
            return err;
        }
        if (th_request_store_query_param(request, key, value) != TH_ERR_OK) {
            return TH_ERR_BAD_ALLOC;
        }
    }
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_request_parse_path(th_request* request, th_string path)
{
    th_err err = TH_ERR_OK;
    size_t query_start = th_string_find_first(path, 0, '?');
    if (query_start == th_string_npos || query_start == path.len - 1) {
        if ((err = th_request_store_uri_path(request, path)) != TH_ERR_OK)
            return err;
        if ((err = th_request_store_uri_query(request, TH_STRING(""))) != TH_ERR_OK)
            return err;
        return TH_ERR_OK;
    }
    if ((err = th_request_store_uri_path(request, th_string_trim(th_string_substr(path, 0, query_start)))) != TH_ERR_OK)
        return err;
    th_string uri_query = th_string_trim(th_string_substr(path, query_start + 1, path.len - query_start - 1));
    if ((err = th_request_store_uri_query(request, uri_query)) != TH_ERR_OK)
        return err;
    return th_request_parse_path_query(request, uri_query);
}

TH_LOCAL(th_err)
th_request_parse_cookie(th_request* request, th_string cookie)
{
    size_t eq = th_string_find_first(cookie, 0, '=');
    if (eq == th_string_npos) {
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    }
    th_string key = th_string_trim(th_string_substr(cookie, 0, eq));
    th_string value = th_string_trim(th_string_substr(cookie, eq + 1, cookie.len));
    th_err err = TH_ERR_OK;
    if ((err = th_request_store_cookie(request, key, value)) != TH_ERR_OK) {
        return err;
    }
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_request_parse_cookie_list(th_request* request, th_string cookie_list)
{
    size_t start = 0;
    size_t pos = 0;
    while (pos != th_string_npos) {
        pos = th_string_find_first(cookie_list, start, ';');
        th_string cookie = th_string_trim(th_string_substr(cookie_list, start, pos - start));
        th_err err = th_request_parse_cookie(request, cookie);
        if (err != TH_ERR_OK) {
            return err;
        }
        start = pos + 1;
    }
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_request_parse_body_params(th_request* request)
{
    th_err err = TH_ERR_OK;
    th_cstr_map_iter iter = th_cstr_map_find(&request->headers, "content-type");
    if (!iter)
        return TH_ERR_OK;
    if (th_cstr_eq(iter->value, "application/x-www-form-urlencoded")) {
        th_buffer body = th_get_body(request);
        size_t pos = 0;
        while (pos != th_string_npos) {
            th_string key;
            th_string value;
            err = th_request_parse_next_query_param(th_string_make(body.ptr, body.len), &pos, &key, &value);
            if (err != TH_ERR_OK) {
                return err;
            }
            if ((err = th_request_store_body_param(request, key, value)) != TH_ERR_OK) {
                return err;
            }
        }
    }
    return err;
}

TH_LOCAL(void)
th_request_read_handle_content(th_request_read_handler* handler, size_t len)
{
    th_request* request = handler->request;
    size_t body_len = request->content_buf_pos + len;
    if (body_len != request->content_buf_len) { // basically means eof, as we requested exact read
        th_request_read_handler_complete(handler, 0, TH_ERR_EOF);
        return;
    }
    // got the whole body, let's parse it if needed
    if (request->parse_body_params) {
        th_err err = th_request_parse_body_params(request);
        if (err != TH_ERR_OK) {
            th_request_read_handler_complete(handler, 0, err);
            return;
        }
    }
    th_request_read_handler_complete(handler, body_len, TH_ERR_OK);
}

TH_LOCAL(th_err)
th_request_handle_headers(th_request* request, struct phr_header* headers, size_t num_headers)
{
    // indirection array for cookie headers
    // Some clients send multiple cookie headers even though it's not allowed
    size_t cookie_headers[TH_CONFIG_MAX_HEADER_NUM];
    size_t num_cookie_headers = 0;
    // copy headers and parse the most important ones
    th_cstr_map_reserve(&request->headers, num_headers);
    th_err err = TH_ERR_OK;
    for (size_t i = 0; i < num_headers; i++) {
        th_string key = th_string_make(headers[i].name, headers[i].name_len);
        th_string value = th_string_make(headers[i].value, headers[i].value_len);
        th_mut_string_tolower((th_mut_string){th_buf_vec_at(&request->buffer, (size_t)(key.ptr - th_buf_vec_begin(&request->buffer))), key.len});
        th_header_id hid = th_header_id_from_string(key.ptr, key.len);
        switch (hid) {
        case TH_HEADER_ID_CONTENT_LENGTH: {
            unsigned int len;
            if ((err = th_string_to_uint(value, &len)) != TH_ERR_OK) {
                return err;
            }
            request->content_len = len;
            break;
        }
        case TH_HEADER_ID_CONNECTION:
            if (TH_STRING_EQ(value, "close")) {
                request->close = true;
            }
            break;
        case TH_HEADER_ID_RANGE:
            if (request->method == TH_METHOD_GET) {
                return TH_ERR_HTTP(TH_CODE_RANGE_NOT_SATISFIABLE);
            } else {
                return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
            }
        case TH_HEADER_ID_COOKIE:
            // Handle cookie headers later to better use the arena allocator
            if (num_cookie_headers == TH_CONFIG_MAX_HEADER_NUM) {
                return TH_ERR_HTTP(TH_CODE_REQUEST_HEADER_FIELDS_TOO_LARGE);
            }
            cookie_headers[num_cookie_headers++] = i;
            break;
        case TH_HEADER_ID_CONTENT_TYPE:
            if (request->method == TH_METHOD_POST) {
                if (th_string_eq(value, TH_STRING("application/x-www-form-urlencoded"))) {
                    request->parse_body_params = true;
                    //} else if (th_string_eq(th_string_substr(value, 0, th_string_find_first(value, 0, ';')), TH_STRING("multipart/form-data"))) {
                } else {
                    return TH_ERR_HTTP(TH_CODE_UNSUPPORTED_MEDIA_TYPE);
                }
            }
            break;
        case TH_HEADER_ID_TRANSFER_ENCODING:
            return TH_ERR_HTTP(TH_CODE_NOT_IMPLEMENTED);
        default:
            break;
        }
        // store the header
        if ((err = th_request_store_header(request, key, value)) != TH_ERR_OK) {
            return err;
        }
    }

    // parse cookie headers
    for (size_t i = 0; i < num_cookie_headers; i++) {
        th_string value = th_string_make(headers[cookie_headers[i]].value, headers[cookie_headers[i]].value_len);
        if ((err = th_request_parse_cookie_list(request, value)) != TH_ERR_OK) {
            return err;
        }
    }
    return TH_ERR_OK;
}

TH_LOCAL(void)
th_request_read_handle_header(th_request_read_handler* handler, size_t len)
{
    th_request* request = handler->request;
    th_string path;
    th_string method;
    size_t num_headers = TH_CONFIG_MAX_HEADER_NUM;
    struct phr_header headers[TH_CONFIG_MAX_HEADER_NUM];
    size_t data_len = request->data_len + len;
    int pret = phr_parse_request(th_buf_vec_at(&request->buffer, 0), data_len, &method.ptr, &method.len, &path.ptr, &path.len, &request->minor_version, headers, &num_headers, request->data_len);
    request->data_len = data_len;
    if (pret == -1) {
        th_request_read_handler_complete(handler, 0, TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
        return;
    } else if (pret == -2) { // we need more data
        size_t buf_len = th_buf_vec_size(&request->buffer);
        if (data_len == buf_len) {
            if (buf_len == TH_CONFIG_LARGE_HEADER_LEN) {
                // We have reached the maximum header length
                th_request_read_handler_complete(handler, 0, TH_ERR_HTTP(TH_CODE_REQUEST_HEADER_FIELDS_TOO_LARGE));
                return;
            }
            th_buf_vec_resize(&request->buffer, TH_CONFIG_LARGE_HEADER_LEN);
        }
        th_socket_async_read(handler->sock, th_buf_vec_at(&request->buffer, data_len), th_buf_vec_size(&request->buffer) - data_len, (th_io_handler*)th_io_composite_ref(&handler->base));
        return;
    }
    size_t header_len = (size_t)pret;
    TH_LOG_DEBUG("Object %p: Parsed request: %.*s %.*s HTTP/%d.%d", request, (int)method.len, method.ptr, (int)path.len, path.ptr, 1, request->minor_version);

    // find method
    struct th_method_mapping* mm = th_method_mapping_find(method.ptr, method.len);
    if (!mm) {
        th_request_read_handler_complete(handler, 0, TH_ERR_HTTP(TH_CODE_NOT_IMPLEMENTED));
        return;
    }
    request->method = mm->method;

    // find path query
    th_err err = TH_ERR_OK;
    if ((err = th_request_parse_path(request, path)) != TH_ERR_OK) {
        th_request_read_handler_complete(handler, 0, err);
        return;
    }

    // handle headers
    if ((err = th_request_handle_headers(request, headers, num_headers)) != TH_ERR_OK) {
        th_request_read_handler_complete(handler, 0, err);
        return;
    }

    // Get is not allowed to have a body
    if (request->method == TH_METHOD_GET && request->content_len > 0) {
        th_request_read_handler_complete(handler, 0, TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
        return;
    }

    // let's check whether we have all the content
    size_t trailing = data_len - header_len;
    if (request->content_len == trailing) {
        th_buf_vec_resize(&request->buffer, data_len);
        th_buf_vec_shrink_to_fit(&request->buffer);
        request->content_buf = th_buf_vec_at(&request->buffer, header_len);
        request->content_buf_len = request->content_buf_pos = trailing;
        th_request_read_handle_content(handler, 0);
        return;
    } else if (request->content_len == 0) { // trailing is not 0
        th_buf_vec_resize(&request->buffer, data_len);
        th_buf_vec_shrink_to_fit(&request->buffer);
        th_request_read_handler_complete(handler, 0, TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
        return;
    }

    // check whether the content length is ok
    if (request->content_len > TH_CONFIG_MAX_CONTENT_LEN) {
        th_request_read_handler_complete(handler, 0, TH_ERR_HTTP(TH_CODE_PAYLOAD_TOO_LARGE));
        return;
    }

    // we have more content, set up the buffer
    size_t remaining_buf = th_buf_vec_size(&request->buffer) - data_len;
    request->content_buf_pos = data_len - header_len; // content length we have so far
    if (remaining_buf >= request->content_len) {
        request->content_buf = th_buf_vec_at(&request->buffer, header_len);
        request->content_buf_len = remaining_buf;
    } else if (th_buf_vec_size(&request->buffer) >= request->content_len) {
        memmove(th_buf_vec_at(&request->buffer, 0), th_buf_vec_at(&request->buffer, header_len), request->content_buf_pos);
        request->content_buf = th_buf_vec_at(&request->buffer, 0);
        request->content_buf_len = th_buf_vec_size(&request->buffer) - request->content_buf_pos;
    } else {
        th_buf_vec_resize(&request->buffer, request->content_len);
        memmove(th_buf_vec_at(&request->buffer, 0), th_buf_vec_at(&request->buffer, header_len), request->content_buf_pos);
        request->content_buf = th_buf_vec_at(&request->buffer, 0);
        request->content_buf_len = th_buf_vec_size(&request->buffer) - request->content_buf_pos;
    }

    // read the rest of the content
    handler->state = TH_REQUEST_READ_STATE_PAYLOAD;
    char* read_pos = request->content_buf + request->content_buf_pos;
    th_socket_async_read_exact(handler->sock, read_pos,
                               request->content_buf_len - request->content_buf_pos,
                               (th_io_handler*)th_io_composite_ref(&handler->base));
}

TH_LOCAL(void)
th_request_read_handler_fn(void* self, size_t len, th_err err)
{
    th_request_read_handler* handler = self;
    if (err) {
        th_request_read_handler_complete(handler, 0, err);
        return;
    }
    if (len == 0) { // unexpected EOF
        th_request_read_handler_complete(handler, 0, TH_ERR_EOF);
        return;
    }

    switch (handler->state) {
    case TH_REQUEST_READ_STATE_HEADER:
        th_request_read_handle_header(handler, len);
        break;
    case TH_REQUEST_READ_STATE_PAYLOAD:
        th_request_read_handle_content(handler, len);
        break;
    default:
        assert(0 && "Invalid state");
        break;
    }
}

TH_PRIVATE(void)
th_request_async_read(th_socket* sock, th_allocator* allocator, th_request* request, th_io_handler* on_complete)
{
    th_request_read_handler* handler = NULL;
    th_err err = TH_ERR_OK;
    if ((err = th_request_read_handler_create(&handler, allocator, sock, request, on_complete)) != TH_ERR_OK) {
        th_context_dispatch_handler(th_socket_get_context(sock), on_complete, 0, err);
        return;
    }
    if ((err = th_buf_vec_resize(&request->buffer, TH_CONFIG_SMALL_HEADER_LEN)) != TH_ERR_OK) {
        th_context_dispatch_handler(th_socket_get_context(sock), (th_io_handler*)handler, 0, err);
        return;
    }
    th_socket_async_read(sock, th_buf_vec_at(&request->buffer, 0), th_buf_vec_size(&request->buffer), (th_io_handler*)handler);
}

TH_LOCAL(const char*)
th_request_store_string(th_request* request, th_string str)
{
    th_heap_string hstr = {0};
    th_heap_string_init(&hstr, &request->string_allocator.base);
    if (th_heap_string_set(&hstr, str) != TH_ERR_OK) {
        th_heap_string_deinit(&hstr);
        return NULL;
    }
    if (th_heap_string_vec_push_back(&request->heap_strings, hstr) != TH_ERR_OK) {
        th_heap_string_deinit(&hstr);
        return NULL;
    }
    return th_heap_string_data(th_heap_string_vec_end(&request->heap_strings) - 1);
}

TH_LOCAL(const char*)
th_request_store_string_url_decoded(th_request* request, th_string str, th_url_decode_type type)
{
    th_heap_string hstr = {0};
    th_heap_string_init(&hstr, &request->string_allocator.base);
    if (th_url_decode_string(str, &hstr, type) != TH_ERR_OK) {
        th_heap_string_deinit(&hstr);
        return NULL;
    }
    if (th_heap_string_vec_push_back(&request->heap_strings, hstr) != TH_ERR_OK) {
        th_heap_string_deinit(&hstr);
        return NULL;
    }
    return th_heap_string_data(th_heap_string_vec_end(&request->heap_strings) - 1);
}

TH_LOCAL(th_err)
th_request_map_store(th_request* request, th_cstr_map* map, th_string key, th_string value)
{
    const char* k = th_request_store_string(request, key);
    const char* v = th_request_store_string(request, value);
    if (!k || !v)
        return TH_ERR_BAD_ALLOC;
    return th_cstr_map_set(map, k, v);
}

TH_LOCAL(th_err)
th_request_map_store_url_decoded(th_request* request, th_cstr_map* map, th_string key, th_string value, th_url_decode_type type)
{
    const char* k = th_request_store_string_url_decoded(request, key, type);
    const char* v = th_request_store_string_url_decoded(request, value, type);
    if (!k || !v)
        return TH_ERR_BAD_ALLOC;
    return th_cstr_map_set(map, k, v);
}

TH_PRIVATE(th_err)
th_request_store_cookie(th_request* request, th_string key, th_string value)
{
    return th_request_map_store(request, &request->cookies, key, value);
}

TH_PRIVATE(th_err)
th_request_store_header(th_request* request, th_string key, th_string value)
{
    return th_request_map_store(request, &request->headers, key, value);
}

TH_PRIVATE(th_err)
th_request_store_query_param(th_request* request, th_string key, th_string value)
{
    return th_request_map_store_url_decoded(request, &request->query_params, key, value, TH_URL_DECODE_TYPE_QUERY);
}

TH_PRIVATE(th_err)
th_request_store_body_param(th_request* request, th_string key, th_string value)
{
    return th_request_map_store_url_decoded(request, &request->body_params, key, value, TH_URL_DECODE_TYPE_QUERY);
}

TH_PRIVATE(th_err)
th_request_store_path_param(th_request* request, th_string key, th_string value)
{
    return th_request_map_store(request, &request->path_params, key, value);
}

TH_PRIVATE(th_err)
th_request_store_uri_path(th_request* request, th_string path)
{
    const char* p = th_request_store_string(request, path);
    if (!p)
        return TH_ERR_BAD_ALLOC;
    request->uri_path = p;
    return TH_ERR_OK;
}

TH_PRIVATE(th_err)
th_request_store_uri_query(th_request* request, th_string query)
{
    const char* q = th_request_store_string(request, query);
    if (!q)
        return TH_ERR_BAD_ALLOC;
    request->uri_query = q;
    return TH_ERR_OK;
}

TH_PRIVATE(void)
th_request_init(th_request* request, th_allocator* allocator)
{
    request->allocator = allocator ? allocator : th_default_allocator_get();
    request->uri_path = NULL;
    request->uri_query = NULL;
    request->map_arena = th_allocator_alloc(request->allocator, TH_REQUEST_MAP_ARENA_LEN);
    request->vec_arena = th_allocator_alloc(request->allocator, TH_REQUEST_VEC_ARENA_LEN);
    request->string_arena = th_allocator_alloc(request->allocator, TH_REQUEST_STRING_ARENA_LEN);
    th_arena_allocator_init(&request->map_allocator, request->map_arena, TH_REQUEST_MAP_ARENA_LEN, request->allocator);
    th_arena_allocator_init(&request->vec_allocator, request->vec_arena, TH_REQUEST_VEC_ARENA_LEN, request->allocator);
    th_arena_allocator_init(&request->string_allocator, request->string_arena, TH_REQUEST_STRING_ARENA_LEN, request->allocator);
    th_cstr_map_init(&request->cookies, &request->map_allocator.base);
    th_cstr_map_init(&request->headers, &request->map_allocator.base);
    th_cstr_map_init(&request->query_params, &request->map_allocator.base);
    th_cstr_map_init(&request->body_params, &request->map_allocator.base);
    th_cstr_map_init(&request->path_params, &request->map_allocator.base);
    th_heap_string_vec_init(&request->heap_strings, &request->vec_allocator.base);
    th_buf_vec_init(&request->buffer, request->allocator);
    request->data_len = 0;
    request->content_len = 0;
    request->content_buf = NULL;
    request->content_buf_len = 0;
    request->content_buf_pos = 0;
    request->close = false;
    request->parse_body_params = false;
}

TH_PRIVATE(void)
th_request_deinit(th_request* request)
{
    th_buf_vec_deinit(&request->buffer);
    th_heap_string_vec_deinit(&request->heap_strings);
    th_cstr_map_deinit(&request->cookies);
    th_cstr_map_deinit(&request->headers);
    th_cstr_map_deinit(&request->query_params);
    th_cstr_map_deinit(&request->body_params);
    th_cstr_map_deinit(&request->path_params);
    th_allocator_free(request->allocator, request->map_arena);
    th_allocator_free(request->allocator, request->vec_arena);
    th_allocator_free(request->allocator, request->string_arena);
}

/* Public request API begin */

TH_PUBLIC(th_buffer)
th_get_body(const th_request* req)
{
    return (th_buffer){req->content_buf, req->content_buf_pos};
}

TH_PUBLIC(const char*)
th_get_path(const th_request* req)
{
    return req->uri_path;
}

TH_PUBLIC(const char*)
th_get_query(const th_request* req)
{
    return req->uri_query;
}

TH_PUBLIC(const char*)
th_try_get_header(const th_request* req, const char* key)
{
    const char** r = th_cstr_map_try_get(&req->headers, key);
    return r ? *r : NULL;
}

TH_PUBLIC(const char*)
th_try_get_cookie(const th_request* req, const char* key)
{
    const char** r = th_cstr_map_try_get(&req->cookies, key);
    return r ? *r : NULL;
}

TH_PUBLIC(const char*)
th_try_get_query_param(const th_request* req, const char* key)
{
    const char** r = th_cstr_map_try_get(&req->query_params, key);
    return r ? *r : NULL;
}

TH_PUBLIC(const char*)
th_try_get_body_param(const th_request* req, const char* key)
{
    const char** r = th_cstr_map_try_get(&req->body_params, key);
    return r ? *r : NULL;
}

TH_PUBLIC(const char*)
th_try_get_path_param(const th_request* req, const char* key)
{
    const char** r = th_cstr_map_try_get(&req->path_params, key);
    return r ? *r : NULL;
}

TH_PUBLIC(th_method)
th_get_method(const th_request* req)
{
    return req->method;
}

TH_PUBLIC(th_map*)
th_get_headers(const th_request* req)
{
    return (th_map*)&req->headers;
}

TH_PUBLIC(th_map*)
th_get_cookies(const th_request* req)
{
    return (th_map*)&req->cookies;
}

TH_PUBLIC(th_map*)
th_get_query_params(const th_request* req)
{
    return (th_map*)&req->query_params;
}

TH_PUBLIC(th_map*)
th_get_body_params(const th_request* req)
{
    return (th_map*)&req->body_params;
}

TH_PUBLIC(th_map*)
th_get_path_params(const th_request* req)
{
    return (th_map*)&req->path_params;
}

TH_PUBLIC(th_map_iter)
th_map_find(th_map* map, const char* key)
{
    return (th_map_iter)th_cstr_map_find((th_cstr_map*)map, key);
}

TH_PUBLIC(th_map_iter)
th_map_begin(th_map* map)
{
    return (th_map_iter)th_cstr_map_begin((th_cstr_map*)map);
}

TH_PUBLIC(th_map_iter)
th_map_next(th_map* map, th_map_iter iter)
{
    return (th_map_iter)th_cstr_map_next((th_cstr_map*)map, (th_cstr_map_iter)iter);
}

/* Public request API end */
