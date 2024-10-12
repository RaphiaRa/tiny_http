#include "th_conn.h"

#include <th.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "th_allocator.h"
#include "th_log.h"
#include "th_router.h"
#include "th_socket.h"

/* th_conn_observable begin */

TH_LOCAL(void)
th_conn_observable_destroy(void* self)
{
    th_conn_observable* observable = self;
    th_conn_observer_on_deinit(observable->observer, observable);
    observable->destroy(observable);
}

TH_LOCAL(void)
th_conn_observable_init(th_conn_observable* observable,
                        th_socket* (*get_socket)(void* self),
                        th_address* (*get_address)(void* self),
                        void (*start)(void* self),
                        void (*destroy)(void* self),
                        th_conn_observer* observer)
{
    th_conn_init(&observable->base, get_socket, get_address, start, th_conn_observable_destroy);
    th_conn_observer_on_init(observer, observable);
    observable->destroy = destroy;
    observable->observer = observer;
}

/* th_conn_observable end */
/* th_tcp_conn begin */

#undef TH_LOG_TAG
#define TH_LOG_TAG "tcp_conn"

TH_LOCAL(th_socket*)
th_tcp_conn_get_socket(void* self);

TH_LOCAL(th_address*)
th_tcp_conn_get_address(void* self);

TH_LOCAL(void)
th_tcp_conn_start(void* self);

TH_LOCAL(void)
th_tcp_conn_destroy(void* conn);

TH_LOCAL(void)
th_tcp_conn_init(th_tcp_conn* conn, th_context* context,
                 th_conn_upgrader* upgrader,
                 th_conn_observer* observer,
                 th_allocator* allocator)
{
    th_conn_observable_init(&conn->base, th_tcp_conn_get_socket, th_tcp_conn_get_address,
                            th_tcp_conn_start, th_tcp_conn_destroy, observer);
    conn->context = context;
    conn->allocator = allocator ? allocator : th_default_allocator_get();
    conn->upgrader = upgrader;
    th_tcp_socket_init(&conn->socket, context, conn->allocator);
    th_address_init(&conn->addr);
}

TH_PRIVATE(th_err)
th_tcp_conn_create(th_conn** out, th_context* context,
                   th_conn_upgrader* upgrader,
                   th_conn_observer* observer,
                   th_allocator* allocator)
{
    th_tcp_conn* conn = th_allocator_alloc(allocator, sizeof(th_tcp_conn));
    if (!conn)
        return TH_ERR_BAD_ALLOC;
    th_tcp_conn_init(conn, context, upgrader, observer, allocator);
    *out = (th_conn*)conn;
    return TH_ERR_OK;
}

TH_LOCAL(th_socket*)
th_tcp_conn_get_socket(void* self)
{
    th_tcp_conn* conn = (th_tcp_conn*)self;
    return &conn->socket.base;
}

TH_LOCAL(th_address*)
th_tcp_conn_get_address(void* self)
{
    th_tcp_conn* conn = (th_tcp_conn*)self;
    return &conn->addr;
}

TH_LOCAL(void)
th_tcp_conn_start(void* self)
{
    th_tcp_conn* conn = (th_tcp_conn*)self;
    TH_LOG_TRACE("%p: Starting", conn);
    th_conn_upgrader_upgrade(conn->upgrader, (th_conn*)conn);
}

TH_LOCAL(void)
th_tcp_conn_destroy(void* self)
{
    th_tcp_conn* conn = self;
    TH_LOG_TRACE("%p: Destroying connection", conn);
    th_tcp_socket_deinit(&conn->socket);
    th_allocator_free(conn->allocator, conn);
}

/* th_tcp_conn end */
/* th_ssl_conn begin */

#if TH_WITH_SSL

#undef TH_LOG_TAG
#define TH_LOG_TAG "ssl_conn"

TH_LOCAL(th_socket*)
th_ssl_conn_get_socket(void* self);

TH_LOCAL(th_address*)
th_ssl_conn_get_address(void* self);

TH_LOCAL(void)
th_ssl_conn_start(void* self);

TH_LOCAL(void)
th_ssl_conn_destroy(void* self);

TH_LOCAL(void)
th_ssl_conn_handshake_handler_fn(void* self, size_t len, th_err err)
{
    (void)len;
    th_ssl_conn_io_handler* handler = self;
    th_ssl_conn* conn = handler->conn;
    if (err != TH_ERR_OK) {
        TH_LOG_ERROR("%p Handshake error: %s", conn, th_strerror(err));
        th_conn_destroy((th_conn*)conn);
        return;
    }
    TH_LOG_TRACE("%p Handshake complete", conn);
    th_conn_upgrader_upgrade(conn->upgrader, (th_conn*)conn);
}

TH_LOCAL(void)
th_ssl_conn_shutdown_handler_fn(void* self, size_t len, th_err err)
{
    (void)len;
    th_ssl_conn_io_handler* handler = self;
    th_ssl_conn* conn = handler->conn;
    // Whatever the result, we should finish the connection
    if (err != TH_ERR_OK) {
        TH_LOG_ERROR("%p Shutdown error: %s", conn, th_strerror(err));
    } else {
        TH_LOG_DEBUG("%p Shutdown complete", conn);
    }
    th_ssl_conn_destroy(conn);
}

TH_LOCAL(void)
th_ssl_conn_io_handler_init(th_ssl_conn_io_handler* handler, th_ssl_conn* conn,
                            void (*fn)(void* self, size_t len, th_err err), void (*destroy)(void* self))
{
    th_io_handler_init(&handler->base, fn, destroy);
    handler->conn = conn;
}

TH_LOCAL(th_err)
th_ssl_conn_init(th_ssl_conn* conn, th_context* context, th_ssl_context* ssl_context,
                 th_conn_upgrader* upgrader, th_conn_observer* observer,
                 th_allocator* allocator)
{
    th_conn_observable_init(&conn->base, th_ssl_conn_get_socket, th_ssl_conn_get_address,
                            th_ssl_conn_start, th_ssl_conn_destroy, observer);
    th_ssl_conn_io_handler_init(&conn->handshake_handler, conn,
                                th_ssl_conn_handshake_handler_fn, NULL);
    th_ssl_conn_io_handler_init(&conn->shutdown_handler, conn,
                                th_ssl_conn_shutdown_handler_fn, NULL);
    conn->context = context;
    conn->allocator = allocator;
    conn->upgrader = upgrader;
    th_address_init(&conn->addr);
    return th_ssl_socket_init(&conn->socket, context, ssl_context, conn->allocator);
}

TH_PRIVATE(th_err)
th_ssl_conn_create(th_conn** out, th_context* context, th_ssl_context* ssl_context,
                   th_conn_upgrader* upgrader,
                   th_conn_observer* observer,
                   th_allocator* allocator)
{
    th_ssl_conn* conn = th_allocator_alloc(allocator, sizeof(th_ssl_conn));
    if (!conn)
        return TH_ERR_BAD_ALLOC;
    th_err err = TH_ERR_OK;
    if ((err = th_ssl_conn_init(conn, context, ssl_context, upgrader, observer, allocator)) != TH_ERR_OK) {
        th_allocator_free(allocator, conn);
        return err;
    }
    *out = (th_conn*)conn;
    return TH_ERR_OK;
}

TH_LOCAL(th_socket*)
th_ssl_conn_get_socket(void* self)
{
    th_ssl_conn* conn = (th_ssl_conn*)self;
    return (th_socket*)&conn->socket;
}

TH_LOCAL(th_address*)
th_ssl_conn_get_address(void* self)
{
    th_ssl_conn* conn = (th_ssl_conn*)self;
    return &conn->addr;
}

TH_LOCAL(void)
th_ssl_conn_start(void* self)
{
    th_ssl_conn* conn = (th_ssl_conn*)self;
    TH_LOG_TRACE("%p: Starting", conn);
    th_ssl_socket_set_mode(&conn->socket, TH_SSL_SOCKET_MODE_SERVER);
    th_ssl_socket_async_handshake(&conn->socket, &conn->handshake_handler.base);
}

TH_LOCAL(void)
th_ssl_conn_destroy(void* self)
{
    th_ssl_conn* conn = self;
    TH_LOG_TRACE("%p Destroying connection", conn);
    th_ssl_socket_deinit(&conn->socket);
    th_allocator_free(conn->allocator, conn);
}

#endif /* TH_WITH_SSL */

/* th_ssl_conn end */
