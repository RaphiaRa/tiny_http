#include "th_tcp_conn.h"

#include "th_log.h"
#include "th_recv.h"
#include "th_sendfile.h"
#include "th_sendvec.h"
#include "th_socket.h"

#undef TH_LOG_TAG
#define TH_LOG_TAG "tcp_conn"

/** th_tcp_conn_op
 * @brief At most one recv and one send are ever in flight at a time on
 * an HTTP connection (request read, then response write), so a single
 * union covers every th_conn_methods.recv/send call without allocating.
 */
typedef union th_tcp_conn_op {
    th_recv_op recv;
    th_sendvec_op sendvec;
    th_sendfile_op sendfile;
} th_tcp_conn_op;

typedef struct th_tcp_conn {
    th_conn_observable base;
    th_socket socket;
    th_address addr;
    th_tcp_conn_op recv_op;
    th_tcp_conn_op send_op;
    th_conn_upgrader* upgrader;
    th_allocator* allocator;
} th_tcp_conn;

TH_LOCAL(th_address*)
th_tcp_conn_get_address(void* self)
{
    th_tcp_conn* conn = self;
    return &conn->addr;
}

TH_LOCAL(th_socket*)
th_tcp_conn_get_socket(void* self)
{
    th_tcp_conn* conn = self;
    return &conn->socket;
}

TH_LOCAL(void)
th_tcp_conn_start(void* self)
{
    th_tcp_conn* conn = self;
    TH_LOG_TRACE("%p: Starting", conn);
    th_conn_upgrader_upgrade(conn->upgrader, (th_conn*)conn);
}

TH_LOCAL(void)
th_tcp_conn_recv(void* self, void* addr, size_t len, bool exact, th_recv_cb callback, void* user_data)
{
    th_tcp_conn* conn = self;
    th_recv_op_init(&conn->recv_op.recv, &conn->socket, addr, len, exact, callback, user_data);
    th_op_perform(&conn->recv_op.recv.base);
}

TH_LOCAL(void)
th_tcp_conn_send(void* self, th_iov* iov, size_t iovcnt, th_file* file, size_t offset, size_t len, th_send_cb callback, void* user_data)
{
    th_tcp_conn* conn = self;
    if (file) {
        th_sendfile_op_init(&conn->send_op.sendfile, &conn->socket, iov, iovcnt, file, offset, len, callback, user_data);
        th_op_perform(&conn->send_op.sendfile.base);
    } else {
        th_sendvec_op_init(&conn->send_op.sendvec, &conn->socket, iov, iovcnt, callback, user_data);
        th_op_perform(&conn->send_op.sendvec.base);
    }
}

TH_LOCAL(void)
th_tcp_conn_cancel(void* self)
{
    th_tcp_conn* conn = self;
    th_socket_cancel(&conn->socket);
}

TH_LOCAL(void)
th_tcp_conn_free(void* self)
{
    th_tcp_conn* conn = self;
    TH_LOG_TRACE("%p: Destroying connection", conn);
    th_socket_deinit(&conn->socket);
    th_allocator_free(conn->allocator, conn);
}

static const th_conn_methods th_tcp_conn_methods = {
    .get_address = th_tcp_conn_get_address,
    .get_socket = th_tcp_conn_get_socket,
    .start = th_tcp_conn_start,
    .recv = th_tcp_conn_recv,
    .send = th_tcp_conn_send,
    .cancel = th_tcp_conn_cancel,
    .destroy = th_conn_observable_destroy,
};

TH_PRIVATE(th_err)
th_tcp_conn_create(th_conn** out, th_socket* socket,
                   th_conn_upgrader* upgrader, th_conn_observer* observer,
                   th_allocator* allocator)
{
    allocator = allocator ? allocator : th_default_allocator_get();
    th_tcp_conn* conn = th_allocator_alloc(allocator, sizeof(th_tcp_conn));
    if (!conn)
        return TH_ERR_BAD_ALLOC;
    th_conn_observable_init(&conn->base, &th_tcp_conn_methods, th_tcp_conn_free, observer);
    conn->upgrader = upgrader;
    conn->allocator = allocator;
    conn->socket = *socket;
    th_address_init(&conn->addr);
    *out = (th_conn*)conn;
    return TH_ERR_OK;
}
