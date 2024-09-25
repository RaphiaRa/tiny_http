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
    response->allocator = allocator;
    response->last_chunk_type = TH_CHUNK_TYPE_NONE;
    response->code = TH_CODE_OK;
    response->is_file = 0;
    response->fcache = fcache;
    response->fcache_entry = NULL;
    response->cur_header_buf_pos = 0;
    response->cur_header_buf_len = 0;

    // First buffer is for the start line
    response->iov[0].base = NULL;
    response->iov[0].len = 0;

    // 1..n buffers for the headers
    response->header_buf = &response->iov[1];
    response->header_buf[0].base = NULL;
    response->header_buf[0].len = 0;
    th_heap_string_init(&response->body, allocator);

    memset(response->header_is_set, 0, sizeof(response->header_is_set));
}

TH_PRIVATE(void)
th_response_deinit(th_response* response)
{
    if (response->iov[0].base) {
        th_allocator_free(response->allocator, response->iov[0].base);
    }
    for (size_t i = 0; i < response->cur_header_buf_pos; ++i) {
        th_allocator_free(response->allocator, response->header_buf[i].base);
    }
    th_heap_string_deinit(&response->body);
    if (response->fcache_entry) {
        th_fcache_entry_unref(response->fcache_entry);
        response->fcache_entry = NULL;
    }
}

TH_PRIVATE(void)
th_response_set_code(th_response* response, th_code code)
{
    response->code = code;
}

static th_err
th_response_increase_cur_header_buf(th_response* response, size_t new_len)
{
    size_t pos = response->cur_header_buf_pos;
    void* new_buf = th_allocator_realloc(response->allocator, response->header_buf[pos].base, new_len);
    if (!new_buf) {
        return TH_ERR_BAD_ALLOC;
    }
    response->header_buf[pos].base = new_buf;
    response->cur_header_buf_len = new_len;
    return TH_ERR_OK;
}

TH_PUBLIC(th_err)
th_response_add_header(th_response* response, th_string key, th_string value)
{
    size_t pos = response->cur_header_buf_pos;
    if (pos == TH_RESPONSE_MAX_IOV) {
        return TH_ERR_BAD_ALLOC;
    }
    th_header_id header_id = th_header_id_from_string(key.ptr, key.len);
    if (header_id != TH_HEADER_ID_UNKNOWN && response->header_is_set[header_id]) {
        return TH_ERR_INVALID_ARG;
    }

    size_t header_len = key.len + value.len + 4;
    size_t header_buf_len = header_len + 1; // +1 for null terminator
    if (!response->header_buf[pos].base) {
        size_t buf_len = 2 * header_buf_len;
        response->header_buf[pos].base = th_allocator_alloc(response->allocator, buf_len);
        if (!response->header_buf[pos].base) {
            return TH_ERR_BAD_ALLOC;
        }
        response->cur_header_buf_len = buf_len;
    } else if ((response->header_buf[pos].len + header_buf_len) > response->cur_header_buf_len) {
        size_t new_len = 2 * response->cur_header_buf_len;
        th_err err = TH_ERR_OK;
        if ((err = th_response_increase_cur_header_buf(response, new_len)) != TH_ERR_OK) {
            return err;
        }
    }

    // Now we have enough space to write the header.
    char* buf = (char*)response->header_buf[pos].base + response->header_buf[pos].len;
    size_t buf_pos = 0;
    buf_pos += th_fmt_strn_append(buf, buf_pos, header_buf_len, key.ptr, key.len);
    buf_pos += th_fmt_str_append(buf, buf_pos, header_buf_len, ": ");
    buf_pos += th_fmt_strn_append(buf, buf_pos, header_buf_len, value.ptr, value.len);
    buf_pos += th_fmt_str_append(buf, buf_pos, header_buf_len, "\r\n");
    response->header_buf[pos].len += buf_pos;
    response->last_chunk_type = TH_CHUNK_TYPE_HEADER;
    if (header_id != TH_HEADER_ID_UNKNOWN) {
        response->header_is_set[header_id] = 1;
    }
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_response_finalize_header_buf(th_response* response)
{
    // Finalize the last header buffer
    // Shrink the buffer to the actual needed size
    size_t pos = response->cur_header_buf_pos;
    void* new_buf = th_allocator_realloc(response->allocator, response->header_buf[pos].base, response->header_buf[pos].len);
    assert(new_buf && "Reallocation with smaller size should always succeed");
    if (!new_buf) {
        return TH_ERR_BAD_ALLOC;
    }
    response->header_buf[pos].base = new_buf;
    response->cur_header_buf_pos++;
    response->header_buf[response->cur_header_buf_pos].base = NULL;
    response->header_buf[response->cur_header_buf_pos].len = 0;
    return TH_ERR_OK;
}

static th_string
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

TH_LOCAL(th_err)
th_response_set_body(th_response* response, th_string body)
{
    if (response->last_chunk_type == TH_CHUNK_TYPE_HEADER) {
        th_response_finalize_header_buf(response);
    }
    th_err err = TH_ERR_OK;
    if ((err = th_heap_string_set(&response->body, body)) != TH_ERR_OK)
        return err;
    response->is_file = 0;
    response->last_chunk_type = TH_CHUNK_TYPE_BODY;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_response_set_body_va(th_response* response, const char* fmt, va_list args)
{
    if (response->last_chunk_type == TH_CHUNK_TYPE_HEADER) {
        th_response_finalize_header_buf(response);
    }
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
        vsnprintf(th_heap_string_data(&response->body), len, fmt, args);
    }
    response->is_file = 0;
    response->last_chunk_type = TH_CHUNK_TYPE_BODY;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_response_set_start_line(th_response* response)
{
    size_t default_header_buf_len = 256;
    char* ptr = th_allocator_alloc(response->allocator, default_header_buf_len);
    if (!ptr) {
        return TH_ERR_BAD_ALLOC;
    }
    char int_buffer[256]; // Buffer for the integer to string conversion
    size_t pos = 0;
    pos += th_fmt_str_append(ptr, pos, default_header_buf_len, "HTTP/1.1 ");
    pos += th_fmt_str_append(ptr, pos, default_header_buf_len, th_fmt_uint_to_str(int_buffer, sizeof(int_buffer), response->code));
    pos += th_fmt_str_append(ptr, pos, default_header_buf_len, " ");
    pos += th_fmt_str_append(ptr, pos, default_header_buf_len, th_http_strerror(response->code));
    ptr[pos++] = '\r';
    ptr[pos++] = '\n';
    response->iov[0].base = ptr;
    response->iov[0].len = (size_t)pos;
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

TH_LOCAL(th_err)
th_response_set_end_of_headers(th_response* response)
{
    size_t pos = response->cur_header_buf_pos;
    if (response->cur_header_buf_len - response->header_buf[pos].len < 2) {
        th_err err = TH_ERR_OK;
        if ((err = th_response_increase_cur_header_buf(response, response->cur_header_buf_len + 2)) != TH_ERR_OK) {
            return err;
        }
    }
    char* buf = response->header_buf[pos].base;
    buf[response->header_buf[pos].len++] = '\r';
    buf[response->header_buf[pos].len++] = '\n';
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
    if ((err = th_response_set_end_of_headers(response)) != TH_ERR_OK)
        goto cleanup;
    if ((err = th_response_finalize_header_buf(response)) != TH_ERR_OK)
        goto cleanup;
    if ((err = th_response_set_start_line(response)) != TH_ERR_OK)
        goto cleanup;
    size_t iovcnt = response->cur_header_buf_pos + 1;
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
