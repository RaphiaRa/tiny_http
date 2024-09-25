#include "th_socket.h"
#include "th_io_composite.h"

#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

/* th_address functions begin */

TH_PRIVATE(void)
th_address_init(th_address* addr)
{
    addr->addrlen = sizeof(addr->addr);
}

/* th_address functions end */
/* generic socket functions begin */

typedef struct th_socket_exact_task_handler {
    th_io_composite base;
    th_allocator* allocator;
    th_socket* socket;
    void* addr;
    size_t remaining;
    size_t len;
} th_socket_exact_task_handler;

TH_LOCAL(void)
th_socket_exact_task_handler_destroy(void* self)
{
    th_socket_exact_task_handler* handler = self;
    th_allocator_free(handler->allocator, handler);
}

TH_LOCAL(void)
th_socket_exact_task_handler_complete(th_socket_exact_task_handler* handler, size_t len, th_err err)
{
    th_io_composite_complete(&handler->base, len, err);
}

/* th_socket_write_exact implementation begin */

typedef th_socket_exact_task_handler th_socket_write_exact_handler;
#define th_socket_write_exact_handler_complete th_socket_exact_task_handler_complete
#define th_socket_write_exact_handler_destroy th_socket_exact_task_handler_destroy

TH_LOCAL(void)
th_socket_write_exact_handler_fn(void* self, size_t len, th_err err)
{
    th_socket_write_exact_handler* handler = self;
    if (err != TH_ERR_OK) {
        th_socket_write_exact_handler_complete(handler, handler->len - handler->remaining, err);
        return;
    }
    handler->remaining -= len;
    if (handler->remaining == 0) {
        th_socket_write_exact_handler_complete(handler, handler->len, err);
        return;
    }
    th_socket_async_write(handler->socket, (uint8_t*)handler->addr + handler->len - handler->remaining, handler->remaining, (th_io_handler*)th_io_composite_ref(&handler->base));
}

TH_LOCAL(th_err)
th_socket_write_exact_handler_create(th_socket_write_exact_handler** out, th_allocator* allocator,
                                     th_socket* socket, void* addr, size_t len, th_socket_handler* on_complete)
{
    th_socket_write_exact_handler* handler = th_allocator_alloc(allocator, sizeof(th_socket_write_exact_handler));
    if (!handler) {
        return TH_ERR_BAD_ALLOC;
    }
    th_io_composite_init(&handler->base, th_socket_write_exact_handler_fn, th_socket_write_exact_handler_destroy, on_complete);
    handler->allocator = allocator;
    handler->socket = socket;
    handler->addr = addr;
    handler->remaining = len;
    handler->len = len;
    *out = handler;
    return TH_ERR_OK;
}

TH_PRIVATE(void)
th_socket_async_write_exact(th_socket* sock, void* addr, size_t len, th_socket_handler* on_complete)
{
    th_err err = TH_ERR_OK;
    th_socket_write_exact_handler* handler = NULL;
    if ((err = th_socket_write_exact_handler_create(&handler, th_socket_get_allocator(sock),
                                                    sock, addr, len, on_complete))
        != TH_ERR_OK) {
        th_context_dispatch_handler(th_socket_get_context(sock), on_complete, 0, err);
        return;
    }
    th_socket_async_write(sock, addr, len, (th_io_handler*)handler);
}

/* th_socket_write_exact implementation end */
/* th_socket_writev_exact implementation begin */

typedef th_socket_exact_task_handler th_socket_writev_exact_handler;
#define th_socket_writev_exact_handler_complete th_socket_exact_task_handler_complete
#define th_socket_writev_exact_handler_destroy th_socket_exact_task_handler_destroy

/** th_socket_writev_exact_handler_fn
 * @brief For each write, shifts the iov array and increases the len by the number of bytes written.
 * The remaining parameter is decremented by the number of buffers consumed.
 */
TH_LOCAL(void)
th_socket_writev_exact_handler_fn(void* self, size_t len, th_err err)
{
    th_socket_exact_task_handler* handler = self;
    if (err != TH_ERR_OK) {
        th_socket_exact_task_handler_complete(handler, handler->len, err);
        return;
    }
    handler->len += len;
    th_iov* iov = handler->addr;
    th_iov_consume(&iov, &handler->remaining, len);
    if (handler->remaining == 0) {
        th_socket_exact_task_handler_complete(handler, handler->len, err);
        return;
    }
    handler->addr = iov;
    th_socket_async_writev(handler->socket, handler->addr, handler->remaining, (th_io_handler*)th_io_composite_ref(&handler->base));
}

TH_LOCAL(th_err)
th_socket_writev_exact_handler_create(th_socket_writev_exact_handler** out, th_allocator* allocator,
                                      th_socket* socket, th_iov* iov, size_t len, th_socket_handler* on_complete)
{
    th_socket_writev_exact_handler* handler = th_allocator_alloc(allocator, sizeof(th_socket_writev_exact_handler));
    if (!handler) {
        return TH_ERR_BAD_ALLOC;
    }
    th_io_composite_init(&handler->base, th_socket_writev_exact_handler_fn, th_socket_writev_exact_handler_destroy, on_complete);
    handler->allocator = allocator;
    handler->socket = socket;
    handler->addr = iov;
    handler->remaining = len;
    handler->len = 0;
    *out = handler;
    return TH_ERR_OK;
}

TH_PRIVATE(void)
th_socket_async_writev_exact(th_socket* sock, th_iov* iov, size_t len, th_socket_handler* on_complete)
{
    th_err err = TH_ERR_OK;
    th_socket_writev_exact_handler* handler = NULL;
    if ((err = th_socket_writev_exact_handler_create(&handler, th_socket_get_allocator(sock), sock, iov, len, on_complete))
        != TH_ERR_OK) {
        th_context_dispatch_handler(th_socket_get_context(sock), on_complete, 0, err);
        return;
    }
    th_socket_async_writev(sock, iov, len, (th_io_handler*)handler);
}

/* th_socket_writev_exact implementation end */
/* th_socket_read_exact implementation begin */

typedef th_socket_exact_task_handler th_socket_read_exact_handler;
#define th_socket_read_exact_handler_complete th_socket_exact_task_handler_complete
#define th_socket_read_exact_handler_destroy th_socket_exact_task_handler_destroy

TH_LOCAL(void)
th_socket_read_exact_handler_fn(void* self, size_t len, th_err err)
{
    th_socket_exact_task_handler* handler = self;
    if (err != TH_ERR_OK) {
        th_socket_read_exact_handler_complete(handler, handler->len - handler->remaining, err);
        return;
    }
    handler->remaining -= len;
    if (handler->remaining == 0) {
        th_socket_read_exact_handler_complete(handler, handler->len, err);
        return;
    }
    th_socket_async_read(handler->socket, (uint8_t*)handler->addr + handler->len - handler->remaining, handler->remaining, (th_io_handler*)th_io_composite_ref(&handler->base));
}

TH_LOCAL(th_err)
th_socket_read_exact_handler_create(th_socket_read_exact_handler** out, th_allocator* allocator,
                                    th_socket* socket, void* addr, size_t len, th_socket_handler* on_complete)
{
    th_socket_read_exact_handler* handler = th_allocator_alloc(allocator, sizeof(th_socket_read_exact_handler));
    if (!handler) {
        return TH_ERR_BAD_ALLOC;
    }
    th_io_composite_init(&handler->base, th_socket_read_exact_handler_fn, th_socket_read_exact_handler_destroy, on_complete);
    handler->allocator = allocator;
    handler->socket = socket;
    handler->addr = addr;
    handler->remaining = len;
    handler->len = len;
    *out = handler;
    return TH_ERR_OK;
}

TH_PRIVATE(void)
th_socket_async_read_exact(th_socket* sock, void* addr, size_t len, th_socket_handler* on_complete)
{
    th_err err = TH_ERR_OK;
    th_socket_read_exact_handler* handler = NULL;
    if ((err = th_socket_read_exact_handler_create(&handler, th_socket_get_allocator(sock), sock, addr, len, on_complete))
        != TH_ERR_OK) {
        th_context_dispatch_handler(th_socket_get_context(sock), on_complete, 0, err);
        return;
    }
    th_socket_async_read(sock, addr, len, (th_io_handler*)handler);
}

/* th_socket_read_exact implementation end */
/* th_socket_readv_exact implementation begin */

typedef th_socket_exact_task_handler th_socket_readv_exact_handler;
#define th_socket_readv_exact_handler_complete th_socket_exact_task_handler_complete
#define th_socket_readv_exact_handler_destroy th_socket_exact_task_handler_destroy

TH_LOCAL(void)
th_socket_readv_exact_handler_fn(void* self, size_t len, th_err err)
{
    th_socket_exact_task_handler* handler = self;
    if (err != TH_ERR_OK) {
        th_socket_readv_exact_handler_complete(handler, handler->len, err);
        return;
    }
    handler->len += len;
    th_iov* iov = handler->addr;
    th_iov_consume(&iov, &handler->remaining, len);
    if (handler->remaining == 0) {
        th_socket_readv_exact_handler_complete(handler, handler->len, err);
        return;
    }
    handler->addr = iov;
    th_socket_async_readv(handler->socket, handler->addr, handler->remaining, (th_io_handler*)th_io_composite_ref(&handler->base));
}

TH_LOCAL(th_err)
th_socket_readv_exact_handler_create(th_socket_readv_exact_handler** out, th_allocator* allocator,
                                     th_socket* socket, th_iov* iov, size_t len, th_socket_handler* on_complete)
{
    th_socket_readv_exact_handler* handler = th_allocator_alloc(allocator, sizeof(th_socket_readv_exact_handler));
    if (!handler) {
        return TH_ERR_BAD_ALLOC;
    }
    th_io_composite_init(&handler->base, th_socket_readv_exact_handler_fn, th_socket_readv_exact_handler_destroy, on_complete);
    handler->allocator = allocator;
    handler->socket = socket;
    handler->addr = iov;
    handler->remaining = len;
    handler->len = 0;
    *out = handler;
    return TH_ERR_OK;
}

TH_PRIVATE(void)
th_socket_async_readv_exact(th_socket* sock, th_iov* iov, size_t len, th_socket_handler* on_complete)
{
    th_err err = TH_ERR_OK;
    th_socket_readv_exact_handler* handler = NULL;
    if ((err = th_socket_readv_exact_handler_create(&handler, th_socket_get_allocator(sock), sock, iov, len, on_complete))
        != TH_ERR_OK) {
        th_context_dispatch_handler(th_socket_get_context(sock), on_complete, 0, err);
        return;
    }
    th_socket_async_readv(sock, iov, len, (th_io_handler*)handler);
}

/* th_socket_readv_exact implementation end */
/* th_socket_sendfile_exact implementation begin */

typedef struct th_socket_sendfile_exact_handler {
    th_io_composite base;
    th_socket* socket;
    th_file* fstream;
    th_iov* iov;
    size_t iovcnt;
    size_t offset;
    size_t slen;
    size_t vlen;
    size_t relative_offset;
} th_socket_sendfile_exact_handler;

TH_LOCAL(void)
th_socket_sendfile_exact_handler_complete(th_socket_sendfile_exact_handler* handler, size_t len, th_err err)
{
    th_io_composite_complete(&handler->base, len, err);
}

TH_LOCAL(void)
th_socket_sendfile_exact_handler_fn(void* self, size_t len, th_err err)
{
    th_socket_sendfile_exact_handler* handler = self;
    if (err != TH_ERR_OK) {
        th_socket_sendfile_exact_handler_complete(handler, 0, err);
        return;
    }
    if (handler->iovcnt > 0) {
        handler->relative_offset += th_iov_consume(&handler->iov, (size_t*)&handler->iovcnt, len);
    } else {
        handler->relative_offset += len;
    }
    if (handler->relative_offset == handler->slen) {
        th_socket_sendfile_exact_handler_complete(handler, handler->relative_offset + handler->vlen, err);
        return;
    }
    size_t remaining = handler->slen - handler->relative_offset;
    size_t chunk = remaining > TH_CONFIG_SENDFILE_CHUNK_LEN ? TH_CONFIG_SENDFILE_CHUNK_LEN : remaining;
    th_socket_async_sendfile(handler->socket, handler->iov, handler->iovcnt, handler->fstream,
                             handler->offset + handler->relative_offset, chunk, (th_io_handler*)th_io_composite_ref(&handler->base));
}

TH_LOCAL(void)
th_socket_sendfile_exact_handler_destroy(void* self)
{
    th_socket_sendfile_exact_handler* handler = self;
    th_allocator_free(th_socket_get_allocator(handler->socket), handler);
}

TH_LOCAL(th_err)
th_socket_sendfile_exact_handler_create(th_socket_sendfile_exact_handler** out, th_allocator* allocator,
                                        th_socket* socket, th_iov* iov, size_t iovcnt, th_file* stream,
                                        size_t offset, size_t slen, size_t vlen, th_socket_handler* on_complete)
{
    th_socket_sendfile_exact_handler* handler = th_allocator_alloc(allocator, sizeof(th_socket_sendfile_exact_handler));
    if (!handler) {
        return TH_ERR_BAD_ALLOC;
    }
    th_io_composite_init(&handler->base, th_socket_sendfile_exact_handler_fn, th_socket_sendfile_exact_handler_destroy, on_complete);
    handler->socket = socket;
    handler->iov = iov;
    handler->iovcnt = iovcnt;
    handler->fstream = stream;
    handler->offset = offset;
    handler->slen = slen;
    handler->vlen = vlen;
    handler->relative_offset = 0;
    *out = handler;
    return TH_ERR_OK;
}

TH_PRIVATE(void)
th_socket_async_sendfile_exact(th_socket* sock, th_iov* iov, size_t iovcnt, th_file* stream, size_t offset, size_t slen, th_socket_handler* on_complete)
{
    size_t vlen = th_iov_bytes(iov, iovcnt);
    th_err err = TH_ERR_OK;
    th_socket_sendfile_exact_handler* handler = NULL;
    if ((err = th_socket_sendfile_exact_handler_create(&handler, th_socket_get_allocator(sock), sock, iov, iovcnt, stream, offset, slen, vlen, on_complete))
        != TH_ERR_OK) {
        th_context_dispatch_handler(th_socket_get_context(sock), on_complete, 0, err);
        return;
    }
    size_t chunk = slen > TH_CONFIG_SENDFILE_CHUNK_LEN ? TH_CONFIG_SENDFILE_CHUNK_LEN : slen;
    th_socket_async_sendfile(sock, iov, iovcnt, stream, offset, chunk, (th_io_handler*)handler);
}

/* th_socket_sendfile_exact implementation end */
/* generic socket functions end */
