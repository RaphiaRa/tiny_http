#include "th_ssl_socket.h"

#if TH_WITH_SSL
#include "th_io_composite.h"
#include "th_log.h"
#include "th_ssl_error.h"
#include "th_ssl_smem_bio.h"
#include "th_utility.h"

#include <assert.h>
#include <errno.h>

#include <openssl/err.h>

#undef TH_LOG_TAG
#define TH_LOG_TAG "ssl_socket"

#define TH_SSL_STATE_CLEAR INT_MIN

/* th_ssl_socket functions begin */

TH_LOCAL(void)
th_ssl_socket_set_fd_impl(void* self, int fd);

TH_LOCAL(void)
th_ssl_socket_cancel_impl(void* self);

TH_LOCAL(th_allocator*)
th_ssl_socket_get_allocator_impl(void* self);

TH_LOCAL(th_context*)
th_ssl_socket_get_context_impl(void* self);

TH_LOCAL(void)
th_ssl_socket_async_write_impl(void* self, void* addr, size_t len, th_socket_handler* on_complete);

TH_LOCAL(void)
th_ssl_socket_async_writev_impl(void* self, th_iov* addr, size_t len, th_socket_handler* on_complete);

TH_LOCAL(void)
th_ssl_socket_async_read_impl(void* self, void* addr, size_t len, th_socket_handler* on_complete);

TH_LOCAL(void)
th_ssl_socket_async_readv_impl(void* self, th_iov* iov, size_t len, th_socket_handler* on_complete);

TH_LOCAL(void)
th_ssl_socket_async_sendfile_impl(void* self, th_iov* iov, size_t iovcnt, th_file* stream, size_t offset, size_t len, th_socket_handler* on_complete);

TH_PRIVATE(th_err)
th_ssl_socket_init(th_ssl_socket* socket, th_context* context, th_ssl_context* ssl_context, th_allocator* allocator)
{
    socket->base.set_fd = th_ssl_socket_set_fd_impl;
    socket->base.cancel = th_ssl_socket_cancel_impl;
    socket->base.get_allocator = th_ssl_socket_get_allocator_impl;
    socket->base.get_context = th_ssl_socket_get_context_impl;
    socket->base.async_write = th_ssl_socket_async_write_impl;
    socket->base.async_writev = th_ssl_socket_async_writev_impl;
    socket->base.async_read = th_ssl_socket_async_read_impl;
    socket->base.async_readv = th_ssl_socket_async_readv_impl;
    socket->base.async_sendfile = th_ssl_socket_async_sendfile_impl;
    th_tcp_socket_init(&socket->tcp_socket, context, allocator);

    th_err err = TH_ERR_OK;
    socket->ssl = SSL_new(ssl_context->ctx);
    if (!socket->ssl) {
        err = TH_ERR_SSL(SSL_ERROR_SSL);
        goto cleanup_tcp_socket;
    }
    socket->wbio = BIO_new(th_smem_bio(ssl_context));
    if (!socket->wbio) {
        err = TH_ERR_SSL(SSL_ERROR_SSL);
        goto cleanup_ssl;
    }
    socket->rbio = BIO_new(th_smem_bio(ssl_context));
    if (!socket->rbio) {
        err = TH_ERR_SSL(SSL_ERROR_SSL);
        goto cleanup_wbio;
    }
    th_smem_bio_setup_buf(socket->wbio, th_socket_get_allocator(&socket->base), TH_CONFIG_MAX_SSL_WRITE_BUF_LEN);
    th_smem_bio_setup_buf(socket->rbio, th_socket_get_allocator(&socket->base), TH_CONFIG_MAX_SSL_READ_BUF_LEN);
    SSL_set_bio(socket->ssl, socket->rbio, socket->wbio);
    SSL_set_mode(socket->ssl, SSL_MODE_ENABLE_PARTIAL_WRITE);
    return TH_ERR_OK;
cleanup_wbio:
    BIO_free(socket->wbio);
cleanup_ssl:
    SSL_free(socket->ssl);
cleanup_tcp_socket:
    th_tcp_socket_deinit(&socket->tcp_socket);
    if (err == TH_ERR_SSL(SSL_ERROR_SSL))
        th_ssl_log_error_stack();
    return err;
}

TH_LOCAL(void)
th_ssl_socket_set_fd_impl(void* self, int fd)
{
    th_ssl_socket* sock = self;
    th_tcp_socket_set_fd(&sock->tcp_socket, fd);
}

TH_LOCAL(void)
th_ssl_socket_cancel_impl(void* self)
{
    th_ssl_socket* sock = self;
    th_tcp_socket_cancel(&sock->tcp_socket);
}

TH_LOCAL(th_allocator*)
th_ssl_socket_get_allocator_impl(void* self)
{
    th_ssl_socket* sock = self;
    return th_tcp_socket_get_allocator(&sock->tcp_socket);
}

TH_LOCAL(th_context*)
th_ssl_socket_get_context_impl(void* self)
{
    th_ssl_socket* sock = self;
    return th_tcp_socket_get_context(&sock->tcp_socket);
}

TH_PRIVATE(void)
th_ssl_socket_set_mode(th_ssl_socket* socket, th_ssl_socket_mode mode)
{
    if (mode == TH_SSL_SOCKET_MODE_SERVER) {
        SSL_set_accept_state(socket->ssl);
    } else {
        SSL_set_connect_state(socket->ssl);
    }
}

typedef enum th_ssl_io_state {
    TH_SSL_IO_STATE_NONE,
    TH_SSL_IO_STATE_READ,
    TH_SSL_IO_STATE_WRITE,
} th_ssl_io_state;

/* th_ssl_socket helper functions begin */

TH_LOCAL(size_t)
th_ssl_fill_buffer(char* buf, size_t buf_len, th_iov* iov, size_t iov_len)
{
    size_t bufpos = 0;
    for (size_t i = 0; i < iov_len; i++) {
        size_t avail = buf_len - bufpos;
        if (avail == 0)
            break;
        size_t to_copy = TH_MIN(avail, iov[i].len);
        memcpy(buf + bufpos, iov[i].base, to_copy);
        bufpos += to_copy;
    }
    return bufpos;
}

TH_LOCAL(th_err)
th_ssl_socket_write_buffer(th_ssl_socket* s, char* buffer, size_t length, size_t* result)
{
    int ret = SSL_write(s->ssl, buffer, (int)length);
    if (ret > 0) {
        *result = (size_t)ret;
        return TH_ERR_OK;
    } else {
        return TH_ERR_SSL(SSL_get_error(s->ssl, ret));
    }
}

#define TH_SSL_SOCKET_WRITE_BUF_LEN (16 * 1024)
TH_LOCAL(th_err)
th_ssl_socket_writev_with_file(th_ssl_socket* s, th_iov* iov, size_t iovcnt, th_file* stream, size_t offset, size_t len, size_t* result)
{
    char buffer[TH_SSL_SOCKET_WRITE_BUF_LEN];
    size_t iov_total = th_iov_bytes(iov, iovcnt);
    size_t bufpos = th_ssl_fill_buffer(buffer, TH_SSL_SOCKET_WRITE_BUF_LEN, iov, iovcnt);
    if (bufpos < iov_total) { // incomplete write
        return th_ssl_socket_write_buffer(s, buffer, bufpos, result);
    }
    if (stream) {
        size_t bytes_read = 0;
        size_t readlen = TH_MIN(TH_SSL_SOCKET_WRITE_BUF_LEN - bufpos, len);
        th_err err = th_file_read(stream, buffer + bufpos, readlen, offset, &bytes_read);
        if (err != TH_ERR_OK && bufpos == 0) {
            return err;
        }
        bufpos += bytes_read;
    }
    return th_ssl_socket_write_buffer(s, buffer, bufpos, result);
}

TH_LOCAL(th_err)
th_ssl_socket_readv(th_ssl_socket* s, th_iov* iov, size_t len, size_t* out)
{
    th_err err = TH_ERR_OK;
    size_t result = 0;
    for (size_t i = 0; i < len; i++) {
        int ret = SSL_read(s->ssl, iov[i].base, (int)iov[i].len);
        if (ret <= 0) {
            if (result == 0)
                err = TH_ERR_SSL(SSL_get_error(s->ssl, ret));
            break;
        }
        result += (size_t)ret;
        if ((size_t)ret < iov[i].len)
            break;
    }
    *out = result;
    return err;
}

TH_LOCAL(th_err)
th_ssl_socket_handshake(th_ssl_socket* s)
{
    int ret = SSL_do_handshake(s->ssl);
    if (ret == 1) {
        return TH_ERR_OK;
    } else {
        return TH_ERR_SSL(SSL_get_error(s->ssl, ret));
    }
}

/* th_ssl_socket helper functions end */
/* th_ssl_socket_io_handler begin */

/** th_ssl_socket_io_handler
 * @brief I/O handler for SSL socket.
 */
typedef struct th_ssl_socket_io_handler {
    th_io_composite base;
    th_allocator* allocator;
    th_iov buffer;
    th_ssl_socket* socket;
    void (*handle_result)(void* self, size_t result);
    size_t result; // last successful SSL_read/SSL_write/SSL_handshake result
    size_t depth;
    th_ssl_io_state state;
} th_ssl_socket_io_handler;

TH_LOCAL(void)
th_ssl_socket_io_handler_destroy(void* self)
{
    th_ssl_socket_io_handler* handler = self;
    th_allocator_free(handler->allocator, handler);
}

TH_LOCAL(void)
th_ssl_socket_io_handler_complete(th_ssl_socket_io_handler* handler, size_t result, th_err err)
{
    if (handler->depth > 0) {
        th_io_composite_complete((th_io_composite*)handler, result, err);
    } else {
        th_context_dispatch_composite_completion(th_socket_get_context((th_socket*)handler->socket), (th_io_composite*)handler, result, err);
        th_ssl_socket_io_handler_destroy(handler);
    }
}

TH_LOCAL(void)
th_ssl_socket_io_handler_read_fn(th_ssl_socket_io_handler* handler, size_t result, th_err err)
{
    th_ssl_socket* socket = handler->socket;
    if (err != TH_ERR_OK) {
        th_smem_bio_set_eof(socket->rbio);
        th_ssl_socket_io_handler_complete(handler, 0, err);
        return;
    }
    th_smem_bio_inc_write_pos(socket->rbio, result);
    handler->handle_result(handler, handler->result);
}

TH_LOCAL(void)
th_ssl_socket_io_handler_write_fn(th_ssl_socket_io_handler* handler, size_t result, th_err err)
{
    th_ssl_socket* socket = handler->socket;
    if (err != TH_ERR_OK) {
        th_ssl_socket_io_handler_complete(handler, 0, err);
        return;
    }
    th_smem_bio_inc_read_pos(socket->wbio, result);
    handler->handle_result(handler, handler->result);
}

TH_LOCAL(void)
th_ssl_socket_io_handler_fn(void* self, size_t result, th_err err)
{
    th_ssl_socket_io_handler* handler = self;
    ++handler->depth;
    switch (handler->state) {
    case TH_SSL_IO_STATE_READ:
        handler->state = TH_SSL_IO_STATE_NONE; // reset state
        th_ssl_socket_io_handler_read_fn(handler, result, err);
        break;
    case TH_SSL_IO_STATE_WRITE:
        handler->state = TH_SSL_IO_STATE_NONE; // reset state
        th_ssl_socket_io_handler_write_fn(handler, result, err);
        break;
    case TH_SSL_IO_STATE_NONE:
        th_ssl_socket_io_handler_complete(handler, result, err);
        break;
    default:
        TH_ASSERT(0 && "Invalid state");
        break;
    }
}

TH_LOCAL(void)
th_ssl_socket_io_handler_init(th_ssl_socket_io_handler* handler, th_ssl_socket* socket,
                              void (*handle_result)(void* self, size_t len),
                              th_socket_handler* on_complete, th_allocator* allocator)
{
    th_io_composite_init(&handler->base, th_ssl_socket_io_handler_fn, th_ssl_socket_io_handler_destroy, on_complete);
    handler->allocator = allocator;
    handler->socket = socket;
    handler->handle_result = handle_result;
    handler->state = TH_SSL_IO_STATE_NONE;
    handler->depth = 0;
    handler->result = 0;
}

TH_LOCAL(void)
th_ssl_socket_io_handler_writev_with_file(th_ssl_socket_io_handler* handler, th_iov* iov, size_t iovcnt,
                                          th_file* stream, size_t offset, size_t len, th_io_composite_forward_type type)
{
    th_err err = TH_ERR_OK;
    size_t result = 0;
    th_ssl_socket* socket = handler->socket;
    if ((err = th_ssl_socket_writev_with_file(socket, iov, iovcnt,
                                              stream, offset, len, &result))
        != TH_ERR_OK) {
        if (TH_ERR_CODE(err) == SSL_ERROR_WANT_READ) {
            TH_LOG_DEBUG("SSL_write wants read, switching to async read");
            handler->state = TH_SSL_IO_STATE_READ;
            th_smem_bio_get_wbuf(socket->rbio, &handler->buffer);
            th_tcp_socket_async_read(&socket->tcp_socket, handler->buffer.base, handler->buffer.len,
                                     (th_socket_handler*)th_io_composite_forward(&handler->base, type));
        } else {
            th_ssl_socket_io_handler_complete(handler, result, err);
        }
    } else {
        TH_LOG_DEBUG("SSL_write %d bytes", (int)result);
        handler->result = result;
        handler->state = TH_SSL_IO_STATE_WRITE;
        th_smem_bio_get_rdata(socket->wbio, &handler->buffer);
        th_socket_async_write_exact(&socket->tcp_socket.base, handler->buffer.base, handler->buffer.len,
                                    (th_socket_handler*)th_io_composite_forward(&handler->base, type));
    }
}

TH_LOCAL(void)
th_ssl_socket_io_handler_readv(th_ssl_socket_io_handler* handler, th_iov* iov, size_t iovcnt,
                               th_io_composite_forward_type type)
{
    th_err err = TH_ERR_OK;
    size_t result = 0;
    th_ssl_socket* socket = handler->socket;
    if (((err = th_ssl_socket_readv(socket, iov, iovcnt, &result)) != TH_ERR_OK)
        || (BIO_pending(socket->wbio) > 0)) {
        if (BIO_pending(socket->wbio) > 0) {
            th_smem_bio_get_rdata(socket->wbio, &handler->buffer);
            TH_LOG_DEBUG("[th_ssl_socket] SSL_read wants write, switching to async write");
            handler->state = TH_SSL_IO_STATE_WRITE;
            if (result > 0)
                handler->result = result;
            th_socket_async_write_exact(&socket->tcp_socket.base, handler->buffer.base, handler->buffer.len,
                                        (th_socket_handler*)th_io_composite_forward(&handler->base, type));
        } else if (TH_ERR_CODE(err) == SSL_ERROR_WANT_READ) {
            TH_LOG_DEBUG("[th_ssl_socket] SSL_read wants read, switching to async read");
            handler->state = TH_SSL_IO_STATE_READ;
            th_smem_bio_get_wbuf(socket->rbio, &handler->buffer);
            th_tcp_socket_async_read(&socket->tcp_socket, handler->buffer.base, handler->buffer.len,
                                     (th_socket_handler*)th_io_composite_forward(&handler->base, type));
        } else if (TH_ERR_CODE(err) == SSL_ERROR_ZERO_RETURN) {
            TH_LOG_DEBUG("[th_ssl_socket] SSL_read zero return");
            th_ssl_socket_io_handler_complete(handler, 0, TH_ERR_EOF);
        } else {
            th_ssl_log_error_stack();
            th_ssl_socket_io_handler_complete(handler, 0, err);
        }
    } else {
        th_ssl_socket_io_handler_complete(handler, result, TH_ERR_OK);
    }
}

TH_LOCAL(void)
th_ssl_socket_io_handler_handshake(th_ssl_socket_io_handler* handler,
                                   th_io_composite_forward_type type)
{
    th_err err = TH_ERR_OK;
    th_ssl_socket* socket = handler->socket;
    if (((err = th_ssl_socket_handshake(socket)) != TH_ERR_OK)
        || (BIO_pending(socket->wbio) > 0)) {
        if (BIO_pending(socket->wbio) > 0) {
            if (err == TH_ERR_OK)
                handler->result = 1; // handshake done
            th_smem_bio_get_rdata(socket->wbio, &handler->buffer);
            TH_LOG_DEBUG("SSL_handshake wants write, switching to async write");
            handler->state = TH_SSL_IO_STATE_WRITE;
            th_socket_async_write_exact(&socket->tcp_socket.base, handler->buffer.base, handler->buffer.len,
                                        (th_socket_handler*)th_io_composite_forward(&handler->base, type));
        } else if (TH_ERR_CODE(err) == SSL_ERROR_WANT_READ) {
            TH_LOG_DEBUG("SSL_handshake wants read, switching to async read");
            handler->state = TH_SSL_IO_STATE_READ;
            th_smem_bio_get_wbuf(socket->rbio, &handler->buffer);
            th_tcp_socket_async_read(&socket->tcp_socket, handler->buffer.base, handler->buffer.len,
                                     (th_socket_handler*)th_io_composite_forward(&handler->base, type));
        } else if (TH_ERR_CODE(err) == SSL_ERROR_ZERO_RETURN) {
            TH_LOG_DEBUG("SSL_handshake zero return");
            th_ssl_socket_io_handler_complete(handler, 0, TH_ERR_EOF);
        } else {
            th_ssl_log_error_stack();
            th_ssl_socket_io_handler_complete(handler, 0, err);
        }
    } else {
        th_ssl_socket_io_handler_complete(handler, 1, TH_ERR_OK);
    }
}

/* th_ssl_socket_async_writev begin */
/* th_ssl_socket_async_write begin */

TH_LOCAL(void)
th_ssl_socket_async_write_impl(void* self, void* addr, size_t len, th_socket_handler* on_complete)
{
    th_socket* socket = self;
    (void)addr;
    (void)len;
    TH_LOG_ERROR("th_ssl_socket_async_write not implemented");
    th_context_dispatch_handler(th_socket_get_context(socket), on_complete, 0, TH_ERR_NOSUPPORT);
}

/* th_ssl_socket_async_write end */
/* th_ssl_socket_async_writev begin */

typedef struct th_ssl_socket_write_handler {
    th_ssl_socket_io_handler base;
    th_iov* addr;
    size_t len;
} th_ssl_socket_writev_handler;

TH_LOCAL(void)
th_ssl_socket_writev_handler_fn(void* self, size_t result)
{
    th_ssl_socket_writev_handler* handler = self;
    if (result > 0) {
        th_ssl_socket_io_handler_complete(&handler->base, result, TH_ERR_OK);
    } else {
        th_ssl_socket_io_handler_writev_with_file(&handler->base, handler->addr, handler->len, NULL, 0, 0, TH_IO_COMPOSITE_FORWARD_COPY);
    }
}

TH_LOCAL(th_err)
th_ssl_socket_writev_handler_create(th_ssl_socket_writev_handler** out, th_ssl_socket* socket, th_socket_handler* on_complete)
{
    th_allocator* allocator = th_socket_get_allocator((th_socket*)socket);
    th_ssl_socket_writev_handler* handler = th_allocator_alloc(allocator, sizeof(th_ssl_socket_writev_handler));
    if (!handler) {
        return TH_ERR_BAD_ALLOC;
    }
    th_ssl_socket_io_handler_init(&handler->base, socket,
                                  th_ssl_socket_writev_handler_fn, on_complete, allocator);
    handler->addr = NULL;
    handler->len = 0;
    *out = handler;
    return TH_ERR_OK;
}

TH_LOCAL(void)
th_ssl_socket_async_writev_impl(void* self, th_iov* addr, size_t len, th_socket_handler* on_complete)
{
    TH_ASSERT(self);
    TH_ASSERT(addr);
    TH_ASSERT(on_complete);
    th_err err = TH_ERR_OK;
    th_ssl_socket* socket = self;
    th_ssl_socket_writev_handler* handler = NULL;
    if ((err = th_ssl_socket_writev_handler_create(&handler, socket, on_complete)) != TH_ERR_OK) {
        th_context_dispatch_handler(th_socket_get_context(&socket->base), on_complete, 0, err);
        return;
    }
    handler->addr = addr;
    handler->len = len;
    th_ssl_socket_io_handler_writev_with_file(&handler->base, addr, len, NULL, 0, 0, TH_IO_COMPOSITE_FORWARD_MOVE);
}

/* th_ssl_socket_async_writev end */
/* th_ssl_socket_async_read begin */

typedef struct th_ssl_socket_read_handler {
    th_ssl_socket_io_handler base;
    th_iov iov;
} th_ssl_socket_read_handler;

TH_LOCAL(void)
th_ssl_socket_read_handler_fn(void* self, size_t result)
{
    th_ssl_socket_read_handler* handler = self;
    if (result > 0) {
        th_ssl_socket_io_handler_complete(&handler->base, result, TH_ERR_OK);
    } else {
        th_ssl_socket_io_handler_readv(&handler->base, &handler->iov, 1, TH_IO_COMPOSITE_FORWARD_COPY);
    }
}

TH_LOCAL(th_err)
th_ssl_socket_read_handler_create(th_ssl_socket_read_handler** out, th_ssl_socket* socket,
                                  th_socket_handler* on_complete)
{
    th_allocator* allocator = th_socket_get_allocator((th_socket*)socket);
    th_ssl_socket_read_handler* handler = th_allocator_alloc(allocator, sizeof(th_ssl_socket_read_handler));
    if (!handler) {
        return TH_ERR_BAD_ALLOC;
    }
    th_ssl_socket_io_handler_init(&handler->base, socket, th_ssl_socket_read_handler_fn, on_complete, allocator);
    handler->iov = (th_iov){0};
    *out = handler;
    return TH_ERR_OK;
}

TH_LOCAL(void)
th_ssl_socket_async_read_impl(void* self, void* addr, size_t len, th_socket_handler* on_complete)
{
    TH_ASSERT(self);
    TH_ASSERT(addr);
    TH_ASSERT(on_complete);
    th_err err = TH_ERR_OK;
    th_ssl_socket* socket = self;
    th_ssl_socket_read_handler* handler = NULL;
    if ((err = th_ssl_socket_read_handler_create(&handler, socket, on_complete)) != TH_ERR_OK) {
        th_context_dispatch_handler(th_socket_get_context(&socket->base), on_complete, 0, err);
        return;
    }
    handler->iov = (th_iov){.base = addr, .len = len};
    th_ssl_socket_io_handler_readv(&handler->base, &handler->iov, 1, TH_IO_COMPOSITE_FORWARD_MOVE);
}

/* th_ssl_socket_async_read end */
/* th_ssl_socket_async_readv begin */

TH_LOCAL(void)
th_ssl_socket_async_readv_impl(void* self, th_iov* iov, size_t len, th_socket_handler* on_complete)
{
    (void)self;
    (void)iov;
    (void)len;
    (void)on_complete;
    // Don't support readv for now as we don't need it
    TH_ASSERT(0 && "Not implemented");
    return;
}

/* th_ssl_socket_async_readv end */
/* th_ssl_socket_async_sendfile begin */

typedef struct th_ssl_sendfile_handler {
    th_ssl_socket_io_handler base;
    th_iov* headers;
    size_t num_headers;
    th_file* stream;
    size_t offset;
    size_t len;
} th_ssl_socket_sendfile_handler;

TH_LOCAL(void)
th_ssl_socket_sendfile_handler_fn(void* self, size_t result)
{
    th_ssl_socket_sendfile_handler* handler = self;
    if (result > 0) {
        th_ssl_socket_io_handler_complete(&handler->base, result, TH_ERR_OK);
    } else {
        th_ssl_socket_io_handler_writev_with_file(&handler->base, handler->headers, handler->num_headers, handler->stream, handler->offset, handler->len, TH_IO_COMPOSITE_FORWARD_COPY);
    }
}

TH_LOCAL(th_err)
th_ssl_socket_sendfile_handler_create(th_ssl_socket_sendfile_handler** out, th_ssl_socket* socket,
                                      th_socket_handler* on_complete)
{
    th_allocator* allocator = th_socket_get_allocator((th_socket*)socket);
    th_ssl_socket_sendfile_handler* handler = th_allocator_alloc(allocator, sizeof(th_ssl_socket_sendfile_handler));
    if (!handler)
        return TH_ERR_BAD_ALLOC;
    th_ssl_socket_io_handler_init(&handler->base, socket, th_ssl_socket_sendfile_handler_fn, on_complete, allocator);
    handler->headers = NULL;
    handler->num_headers = 0;
    handler->stream = NULL;
    handler->offset = 0;
    *out = handler;
    return TH_ERR_OK;
}

TH_LOCAL(void)
th_ssl_socket_async_sendfile_impl(void* self, th_iov* iov, size_t iovcnt, th_file* stream, size_t offset, size_t len, th_socket_handler* on_complete)
{
    th_err err = TH_ERR_OK;
    th_ssl_socket* sock = self;
    th_ssl_socket_sendfile_handler* handler = NULL;
    if ((err = th_ssl_socket_sendfile_handler_create(&handler, sock, on_complete)) != TH_ERR_OK) {
        th_context_dispatch_handler(th_socket_get_context(self), on_complete, 0, err);
        return;
    }
    handler->headers = iov;
    handler->num_headers = iovcnt;
    handler->stream = stream;
    handler->offset = offset;
    th_ssl_socket_io_handler_writev_with_file(&handler->base, iov, iovcnt, stream, offset, len, TH_IO_COMPOSITE_FORWARD_MOVE);
}

/* th_ssl_socket_async_sendfile end */
/* th_ssl_socket_async_handshake begin */

typedef struct th_ssl_socket_handshake_handler {
    th_ssl_socket_io_handler base;
} th_ssl_socket_handshake_handler;

TH_LOCAL(void)
th_ssl_socket_handshake_handler_fn(void* self, size_t result)
{
    th_ssl_socket_handshake_handler* handler = self;
    if (result > 0) {
        th_ssl_socket_io_handler_complete(&handler->base, result, TH_ERR_OK);
    } else {
        th_ssl_socket_io_handler_handshake(&handler->base, TH_IO_COMPOSITE_FORWARD_COPY);
    }
}

TH_LOCAL(th_err)
th_ssl_socket_handshake_handler_create(th_ssl_socket_handshake_handler** out, th_ssl_socket* socket, th_socket_handler* on_complete)
{
    th_allocator* allocator = th_socket_get_allocator((th_socket*)socket);
    th_ssl_socket_handshake_handler* handler = th_allocator_alloc(allocator, sizeof(th_ssl_socket_handshake_handler));
    if (!handler) {
        return TH_ERR_BAD_ALLOC;
    }
    th_ssl_socket_io_handler_init(&handler->base, socket, th_ssl_socket_handshake_handler_fn, on_complete, allocator);
    *out = handler;
    return TH_ERR_OK;
}

TH_PRIVATE(void)
th_ssl_socket_async_handshake(th_ssl_socket* socket, th_socket_handler* on_complete)
{
    th_err err = TH_ERR_OK;
    th_ssl_socket_handshake_handler* handler = NULL;
    if ((err = th_ssl_socket_handshake_handler_create(&handler, socket, on_complete)) != TH_ERR_OK) {
        th_context_dispatch_handler(th_socket_get_context(&socket->base), on_complete, 0, err);
        return;
    }
    th_smem_ensure_buf_size(socket->rbio, TH_CONFIG_SMALL_SSL_BUF_LEN);
    th_ssl_socket_io_handler_handshake(&handler->base, TH_IO_COMPOSITE_FORWARD_MOVE);
}

/* th_ssl_socket_async_handshake end */
/* th_ssl_socket_async_shutdown begin */

TH_PRIVATE(void)
th_ssl_socket_async_shutdown(th_ssl_socket* socket, th_socket_handler* on_complete)
{
    (void)socket;
    (void)on_complete;
    TH_ASSERT(0 && "Not implemented");
}

/* th_ssl_socket_async_shutdown end */

TH_PRIVATE(void)
th_ssl_socket_close(th_ssl_socket* sock)
{
    th_tcp_socket_close(&sock->tcp_socket);
}

TH_PRIVATE(void)
th_ssl_socket_deinit(th_ssl_socket* sock)
{
    th_ssl_socket_close(sock);
    SSL_free(sock->ssl);
    th_tcp_socket_deinit(&sock->tcp_socket);
}
#endif
