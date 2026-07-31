#include "th_recv.h"
#include "th_system_error.h"

TH_LOCAL(bool)
th_recv_op_is_retryable(th_err err)
{
    return err == TH_ERR_SYSTEM(TH_EAGAIN)
           || err == TH_ERR_SYSTEM(TH_EWOULDBLOCK);
}

TH_LOCAL(void)
th_recv_op_finalize(th_recv_op* op)
{
    op->callback(op->user_data, op->pos, op->err);
}

TH_LOCAL(void)
th_recv_op_complete(th_recv_op* op, th_err err)
{
    op->err = err;
    th_op_set_flags(&op->base, TH_OP_COMPLETED);
    th_socket_post(op->socket, &op->base.base);
}

TH_LOCAL(th_err)
th_recv_op_perform(th_recv_op* op)
{
    th_op_clear_flags(&op->base, TH_OP_IMMEDIATE);
    size_t result = 0;
    th_err err = th_socket_recv(op->socket, (char*)op->addr + op->pos, op->len - op->pos, &result);
    if (err != TH_ERR_OK)
        return err;
    op->pos += result;
    if (!op->exact || op->pos == op->len)
        return TH_ERR_OK;
    return TH_ERR_SYSTEM(TH_EAGAIN);
}

TH_LOCAL(void)
th_recv_op_fn(void* self)
{
    th_recv_op* op = self;
    if (th_op_get_flags(&op->base) & TH_OP_COMPLETED) {
        th_recv_op_finalize(op);
        return;
    }
    th_err err = th_recv_op_perform(op);
    if (th_recv_op_is_retryable(err)) {
        err = th_socket_submit(op->socket, &op->base);
        if (err == TH_ERR_OK)
            return;
    }
    th_recv_op_complete(op, err);
}

TH_LOCAL(void)
th_recv_op_abort(void* self, th_err err)
{
    th_recv_op_complete(self, err);
}

TH_PRIVATE(void)
th_recv_op_init(th_recv_op* op, th_socket* socket, void* addr, size_t len, bool exact, th_recv_cb callback, void* user_data)
{
    th_op_init(&op->base, TH_OP_READ, th_recv_op_fn, NULL, th_recv_op_abort);
    op->socket = socket;
    op->addr = addr;
    op->len = len;
    op->pos = 0;
    op->exact = exact;
    op->callback = callback;
    op->user_data = user_data;
    op->err = TH_ERR_OK;
}
