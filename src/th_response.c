#include <th.h>

#include "th_fmt.h"
#include "th_http_error.h"
#include "th_log.h"
#include "th_mime.h"
#include "th_response.h"
#include "th_string.h"

#include <assert.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#undef TH_LOG_TAG
#define TH_LOG_TAG "response"

/* th_response implementation begin */

TH_PRIVATE(void)
th_response_init(th_response* response, th_fcache* fcache, th_allocator* allocator)
{
    allocator = allocator ? allocator : th_default_allocator_get();
    th_heap_string_init(&response->headers, allocator);
    th_heap_string_init(&response->body, allocator);
    response->iov[0] = (th_iov){0};
    response->iov[1] = (th_iov){0};
    response->iov[2] = (th_iov){0};
    response->allocator = allocator;
    response->fcache = fcache;
    response->fcache_entry = NULL;
    response->file_len = 0;
    response->code = TH_CODE_OK;
    memset(response->header_is_set, 0, sizeof(response->header_is_set));
    response->is_file = false;
    response->only_headers = false;
}

TH_PRIVATE(void)
th_response_deinit(th_response* response)
{
    th_heap_string_deinit(&response->headers);
    th_heap_string_deinit(&response->body);
    if (response->fcache_entry) {
        th_fcache_entry_unref(response->fcache_entry);
        response->fcache_entry = NULL;
    }
}

TH_PRIVATE(void)
th_response_reset(th_response* response)
{
    th_heap_string_clear(&response->headers);
    th_heap_string_clear(&response->body);
    response->iov[0] = (th_iov){0};
    response->iov[1] = (th_iov){0};
    response->iov[2] = (th_iov){0};
    if (response->fcache_entry) {
        th_fcache_entry_unref(response->fcache_entry);
        response->fcache_entry = NULL;
    }
    response->file_len = 0;
    response->code = TH_CODE_OK;
    memset(response->header_is_set, 0, sizeof(response->header_is_set));
    response->is_file = false;
    response->only_headers = false;
}

TH_PRIVATE(void)
th_response_set_code(th_response* response, th_code code)
{
    response->code = code;
}

TH_PUBLIC(th_err)
th_response_add_header(th_response* response, th_string key, th_string value)
{
    th_header_id header_id = th_header_id_from_string(key.ptr, key.len);
    if (header_id != TH_HEADER_ID_UNKNOWN && response->header_is_set[header_id]) {
        return TH_ERR_INVALID_ARG;
    }
    th_err err = TH_ERR_OK;
    size_t old_len = th_heap_string_len(&response->headers);
    if ((err = th_heap_string_append(&response->headers, key)) != TH_ERR_OK)
        goto cleanup;
    if ((err = th_heap_string_append(&response->headers, TH_STRING(": "))) != TH_ERR_OK)
        goto cleanup;
    if ((err = th_heap_string_append(&response->headers, value)) != TH_ERR_OK)
        goto cleanup;
    if ((err = th_heap_string_append(&response->headers, TH_STRING("\r\n"))) != TH_ERR_OK)
        goto cleanup;
    if (header_id != TH_HEADER_ID_UNKNOWN) {
        response->header_is_set[header_id] = 1;
    }
    return TH_ERR_OK;
cleanup:
    th_heap_string_resize(&response->headers, old_len, '\0');
    return err;
}

TH_LOCAL(th_string)
th_response_get_mime_type(th_string filename)
{
    char ext[256];
    size_t ei = 0;
    size_t max = filename.len < sizeof(ext) ? filename.len : sizeof(ext);
    for (size_t i = 0; i < max; ++i) {
        size_t ri = filename.len - i - 1;
        ei = max - i - 1;
        ext[ei] = filename.ptr[ri];
        if (filename.ptr[ri] == '.' || filename.ptr[ri] == '/') {
            break;
        }
    }
    struct th_mime_mapping* mm = NULL;
    if (ext[ei] == '.') {
        mm = th_mime_mapping_find(&ext[ei + 1], max - ei - 1);
        return mm ? mm->mime : TH_STRING("application/octet-stream");
    } else {
        return TH_STRING("application/octet-stream");
    }
}

TH_LOCAL(th_err)
th_response_set_body_from_file(th_response* response, th_string root, th_string path)
{
    th_err err = TH_ERR_OK;
    if ((err = th_fcache_get(response->fcache, root, path, &response->fcache_entry)) != TH_ERR_OK) {
        return err;
    }
    // Set the content type, if not already set
    if (response->header_is_set[TH_HEADER_ID_CONTENT_TYPE] == 0) {
        th_string mime_type = th_response_get_mime_type(path);
        if ((err = th_response_add_header(response, TH_STRING("Content-Type"), mime_type)) != TH_ERR_OK)
            goto cleanup_fcache_entry;
    }
    response->is_file = 1;
    return TH_ERR_OK;
cleanup_fcache_entry:
    th_fcache_entry_unref(response->fcache_entry);
    response->fcache_entry = NULL;
    return err;
}

TH_PRIVATE(th_err)
th_response_set_body(th_response* response, th_string body)
{
    th_err err = TH_ERR_OK;
    if ((err = th_heap_string_set(&response->body, body)) != TH_ERR_OK)
        return err;
    response->is_file = 0;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_response_set_body_va(th_response* response, const char* fmt, va_list args)
{
    char buffer[512];
    th_err err = TH_ERR_OK;
    va_list va;
    va_copy(va, args);
    int len = vsnprintf(buffer, sizeof(buffer), fmt, va);
    va_end(va);
    if (len < 0) {
        return TH_ERR_INVALID_ARG;
    } else if ((size_t)len < sizeof(buffer)) {
        if ((err = th_heap_string_set(&response->body, th_string_make(buffer, (size_t)len))) != TH_ERR_OK) {
            return err;
        }
    } else {
        th_heap_string_resize(&response->body, (size_t)len, ' ');
        vsnprintf(th_heap_string_at(&response->body, 0), len, fmt, args);
    }
    response->is_file = 0;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_response_finalize_headers(th_response* response)
{
    th_err err = TH_ERR_OK;
    if ((err = th_heap_string_append(&response->headers, TH_STRING("\r\n"))) != TH_ERR_OK)
        return err;
    size_t headers_len = th_heap_string_len(&response->headers);

    // Set the start line
    char int_buffer[128]; // Buffer for the integer to string conversion
    if ((err = th_heap_string_append(&response->headers, TH_STRING("HTTP/1.1 "))) != TH_ERR_OK)
        return err;
    if ((err = th_heap_string_append_cstr(&response->headers, th_fmt_uint_to_str(int_buffer, sizeof(int_buffer), response->code))) != TH_ERR_OK)
        return err;
    if ((err = th_heap_string_append(&response->headers, TH_STRING(" "))) != TH_ERR_OK)
        return err;
    if ((err = th_heap_string_append_cstr(&response->headers, th_http_strerror(response->code))) != TH_ERR_OK)
        return err;
    if ((err = th_heap_string_append(&response->headers, TH_STRING("\r\n"))) != TH_ERR_OK)
        return err;
    response->iov[0].base = th_heap_string_at(&response->headers, headers_len);
    response->iov[0].len = th_heap_string_len(&response->headers) - headers_len;
    response->iov[1].base = th_heap_string_at(&response->headers, 0);
    response->iov[1].len = headers_len;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_response_set_default_headers(th_response* response)
{
    th_err err = TH_ERR_OK;
    char buffer[256];
    if (response->is_file) {
        size_t len = 0;
        const char* content_len = th_fmt_uint_to_str_ex(buffer, sizeof(buffer), response->file_len, &len);
        if ((err = th_response_add_header(response, TH_STRING("Content-Length"), th_string_make(content_len, len))) != TH_ERR_OK)
            return err;
    } else {
        size_t len = 0;
        const char* body_len = th_fmt_uint_to_str_ex(buffer, sizeof(buffer), th_heap_string_len(&response->body), &len);
        if ((err = th_response_add_header(response, TH_STRING("Content-Length"), th_string_make(body_len, len))) != TH_ERR_OK)
            return err;
    }
    if (!response->header_is_set[TH_HEADER_ID_SERVER]) {
        if ((err = th_response_add_header(response, TH_STRING("Server"), TH_STRING("TinyHTTP"))) != TH_ERR_OK)
            return err;
    }
    if (!response->header_is_set[TH_HEADER_ID_DATE]) {
        th_date now = th_date_now();
        char date[64];
        size_t len = th_fmt_strtime(date, sizeof(date), now);
        if ((err = th_response_add_header(response, TH_STRING("Date"), th_string_make(date, len))) != TH_ERR_OK)
            return err;
    }
    return TH_ERR_OK;
}

TH_PRIVATE(void)
th_response_async_write(th_response* response, th_socket* socket, th_io_handler* handler)
{
    th_err err = TH_ERR_OK;
    if (response->is_file) {
        response->file_len = response->fcache_entry->stream.size;
    }
    if ((err = th_response_set_default_headers(response)) != TH_ERR_OK)
        goto cleanup;
    if ((err = th_response_finalize_headers(response)) != TH_ERR_OK)
        goto cleanup;
    size_t iovcnt = 2; // start line + headers
    if (response->only_headers) {
        th_socket_async_writev_exact(socket, response->iov, iovcnt, handler);
        return;
    }
    if (response->is_file == 0) { // user provided body
        if (th_heap_string_len(&response->body) > 0) {
            response->iov[iovcnt].base = (void*)th_heap_string_data(&response->body);
            response->iov[iovcnt].len = th_heap_string_len(&response->body);
            iovcnt++;
        }
        th_socket_async_writev_exact(socket, response->iov, iovcnt, handler);
    } else {
        th_socket_async_sendfile_exact(socket, response->iov, iovcnt, &response->fcache_entry->stream, 0, (size_t)response->file_len, handler);
    }
    return;
cleanup:
    th_context_dispatch_handler(th_socket_get_context(socket), handler, 0, err);
}

/* Public response API begin */

TH_PUBLIC(th_err)
th_set_body(th_response* response, const char* body)
{
    return th_response_set_body(response, th_string_from_cstr(body));
}

TH_PUBLIC(th_err)
th_printf_body(th_response* resp, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    th_err err = th_response_set_body_va(resp, fmt, args);
    va_end(args);
    return err;
}

TH_PUBLIC(th_err)
th_set_body_from_file(th_response* response, const char* root, const char* filepath)
{
    (void)root;
    return th_response_set_body_from_file(response, th_string_from_cstr(root), th_string_from_cstr(filepath));
}

TH_PUBLIC(th_err)
th_add_header(th_response* response, const char* key, const char* value)
{
    return th_response_add_header(response, th_string_from_cstr(key), th_string_from_cstr(value));
}

TH_PUBLIC(th_err)
th_add_cookie(th_response* response, const char* key, const char* value, th_cookie_attr* attr)
{
    char buffer[512];
    size_t len = 0;
    len += th_fmt_str_append(buffer, len, sizeof(buffer), key);
    len += th_fmt_str_append(buffer, len, sizeof(buffer), "=");
    len += th_fmt_str_append(buffer, len, sizeof(buffer), value);
    if (attr) {
        th_date empty_date = {0};
        if (memcmp(&attr->expires, &empty_date, sizeof(th_date)) != 0) {
            len += th_fmt_str_append(buffer, len, sizeof(buffer), "; Expires=");
            len += th_fmt_strtime(buffer + len, sizeof(buffer) - len, attr->expires);
        }
        if (attr->max_age.seconds) {
            char max_age[32];
            const char* max_age_str = th_fmt_uint_to_str(max_age, sizeof(max_age), (unsigned int)attr->max_age.seconds);
            len += th_fmt_str_append(buffer, len, sizeof(buffer), "; Max-Age=");
            len += th_fmt_str_append(buffer, len, sizeof(buffer), max_age_str);
        }
        if (attr->domain) {
            len += th_fmt_str_append(buffer, len, sizeof(buffer), "; Domain=");
            len += th_fmt_str_append(buffer, len, sizeof(buffer), attr->domain);
        }
        if (attr->path) {
            len += th_fmt_str_append(buffer, len, sizeof(buffer), "; Path=");
            len += th_fmt_str_append(buffer, len, sizeof(buffer), attr->path);
        }
        if (attr->secure) {
            len += th_fmt_str_append(buffer, len, sizeof(buffer), "; Secure");
        }
        if (attr->http_only) {
            len += th_fmt_str_append(buffer, len, sizeof(buffer), "; HttpOnly");
        }
        if (attr->same_site) {
            len += th_fmt_str_append(buffer, len, sizeof(buffer), "; SameSite=");
            switch (attr->same_site) {
            case TH_COOKIE_SAME_SITE_NONE:
                if (attr->secure) {
                    len += th_fmt_str_append(buffer, len, sizeof(buffer), "None");
                } else {
                    return TH_ERR_INVALID_ARG;
                }
                break;
            case TH_COOKIE_SAME_SITE_LAX:
                len += th_fmt_str_append(buffer, len, sizeof(buffer), "Lax");
                break;
            case TH_COOKIE_SAME_SITE_STRICT:
                len += th_fmt_str_append(buffer, len, sizeof(buffer), "Strict");
                break;
            default:
                return TH_ERR_INVALID_ARG;
                break;
            }
        }
    }
    return th_response_add_header(response, TH_STRING("Set-Cookie"), th_string_make(buffer, len));
}
