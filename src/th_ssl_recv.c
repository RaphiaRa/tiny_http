#include "th_ssl_recv.h"

#if TH_WITH_SSL

TH_LOCAL(void)
th_ssl_recv_op_finalize(th_ssl_recv_op* op, th_err err)
{
    op->callback(op->user_data, op->pos, err);
}

TH_LOCAL(void)
th_ssl_recv_op_start(th_ssl_recv_op* op);

TH_LOCAL(void)
th_ssl_recv_op_io_complete(void* user_data, size_t size, th_err err)
{
    th_ssl_recv_op* op = user_data;
    if (err != TH_ERR_OK) {
        th_ssl_recv_op_finalize(op, err);
        return;
    }
    op->pos += size;
    if (!op->exact || op->pos == op->len) {
        th_ssl_recv_op_finalize(op, TH_ERR_OK);
        return;
    }
    th_ssl_recv_op_start(op);
}

TH_LOCAL(void)
th_ssl_recv_op_start(th_ssl_recv_op* op)
{
    th_ssl_io_op_init_read(&op->io, op->socket, op->session,
                           (char*)op->addr + op->pos, op->len - op->pos,
                           th_ssl_recv_op_io_complete, op);
    th_op_perform(&op->io.base);
}

TH_PRIVATE(void)
th_ssl_recv_op_init(th_ssl_recv_op* op, th_socket* socket, th_ssl_session* session, void* addr, size_t len, bool exact, th_recv_cb callback, void* user_data)
{
    op->socket = socket;
    op->session = session;
    op->addr = addr;
    op->len = len;
    op->pos = 0;
    op->exact = exact;
    op->callback = callback;
    op->user_data = user_data;
    th_ssl_recv_op_start(op);
}

#endif
