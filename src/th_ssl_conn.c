#include "th_ssl_conn.h"

#if TH_WITH_SSL

#include "th_log.h"
#include "th_ssl_recv.h"
#include "th_ssl_send.h"

#undef TH_LOG_TAG
#define TH_LOG_TAG "ssl_conn"

/** th_ssl_conn_op
 * @brief At most one recv and one send are ever in flight at a time on
 * an HTTP connection (request read, then response write), so a single
 * union covers every th_conn_methods.recv/send call without allocating.
 */
typedef union th_ssl_conn_op {
    th_ssl_recv_op recv;
    th_ssl_send_op send;
} th_ssl_conn_op;

typedef struct th_ssl_conn {
    th_conn_observable base;
    th_socket socket;
    th_address addr;
    th_ssl_session session;
    th_ssl_io_op handshake_op;
    th_ssl_conn_op recv_op;
    th_ssl_conn_op send_op;
    th_conn_upgrader* upgrader;
    th_allocator* allocator;
} th_ssl_conn;

TH_LOCAL(th_address*)
th_ssl_conn_get_address(void* self)
{
    th_ssl_conn* conn = self;
    return &conn->addr;
}

TH_LOCAL(th_socket*)
th_ssl_conn_get_socket(void* self)
{
    th_ssl_conn* conn = self;
    return &conn->socket;
}

TH_LOCAL(void)
th_ssl_conn_handshake_complete(void* user_data, size_t size, th_err err)
{
    (void)size;
    th_ssl_conn* conn = user_data;
    if (err != TH_ERR_OK) {
        TH_LOG_ERROR("%p: SSL handshake failed: %s", (void*)conn, th_strerror(err));
        th_conn_destroy((th_conn*)conn);
        return;
    }
    TH_LOG_TRACE("%p: SSL handshake done", conn);
    th_conn_upgrader_upgrade(conn->upgrader, (th_conn*)conn);
}

TH_LOCAL(void)
th_ssl_conn_start(void* self)
{
    th_ssl_conn* conn = self;
    TH_LOG_TRACE("%p: Starting SSL handshake", conn);
    th_ssl_io_op_init_handshake(&conn->handshake_op, &conn->socket, &conn->session,
                                th_ssl_conn_handshake_complete, conn);
    th_op_perform(&conn->handshake_op.base);
}

TH_LOCAL(void)
th_ssl_conn_recv(void* self, void* addr, size_t len, bool exact, th_recv_cb callback, void* user_data)
{
    th_ssl_conn* conn = self;
    th_ssl_recv_op_init(&conn->recv_op.recv, &conn->socket, &conn->session, addr, len, exact, callback, user_data);
}

TH_LOCAL(void)
th_ssl_conn_send(void* self, th_iov* iov, size_t iovcnt, th_file* file, size_t offset, size_t len, th_send_cb callback, void* user_data)
{
    th_ssl_conn* conn = self;
    th_ssl_send_op_init(&conn->send_op.send, &conn->socket, &conn->session, iov, iovcnt, file, offset, len, callback, user_data);
}

TH_LOCAL(void)
th_ssl_conn_cancel(void* self)
{
    th_ssl_conn* conn = self;
    th_socket_cancel(&conn->socket);
}

TH_LOCAL(void)
th_ssl_conn_free(void* self)
{
    th_ssl_conn* conn = self;
    TH_LOG_TRACE("%p: Destroying connection", conn);
    th_ssl_session_deinit(&conn->session);
    th_socket_deinit(&conn->socket);
    th_allocator_free(conn->allocator, conn);
}

static const th_conn_methods th_ssl_conn_methods = {
    .get_address = th_ssl_conn_get_address,
    .get_socket = th_ssl_conn_get_socket,
    .start = th_ssl_conn_start,
    .recv = th_ssl_conn_recv,
    .send = th_ssl_conn_send,
    .cancel = th_ssl_conn_cancel,
    .destroy = th_conn_observable_destroy,
};

TH_PRIVATE(th_err)
th_ssl_conn_create(th_conn** out, th_socket* socket, th_ssl_context* ssl_context, th_ssl_ops* ssl_ops,
                   th_conn_upgrader* upgrader, th_conn_observer* observer,
                   th_allocator* allocator)
{
    allocator = allocator ? allocator : th_default_allocator_get();
    th_ssl_conn* conn = th_allocator_alloc(allocator, sizeof(th_ssl_conn));
    if (!conn)
        return TH_ERR_BAD_ALLOC;
    th_err err = TH_ERR_OK;
    if ((err = th_ssl_session_init(&conn->session, ssl_context, ssl_ops, allocator)) != TH_ERR_OK) {
        th_allocator_free(allocator, conn);
        return err;
    }
    th_conn_observable_init(&conn->base, &th_ssl_conn_methods, th_ssl_conn_free, observer);
    conn->upgrader = upgrader;
    conn->allocator = allocator;
    conn->socket = *socket;
    th_address_init(&conn->addr);
    *out = (th_conn*)conn;
    return TH_ERR_OK;
}

#endif
