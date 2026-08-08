#include "th_send.h"
#include "th_system_error.h"

TH_LOCAL(bool)
th_send_op_is_retryable(th_err err)
{
    return err == TH_ERR_SYSTEM(TH_EAGAIN)
           || err == TH_ERR_SYSTEM(TH_EWOULDBLOCK);
}

TH_LOCAL(void)
th_send_op_finalize(th_send_op* op)
{
    op->callback(op->user_data, op->pos, op->err);
}

TH_LOCAL(void)
th_send_op_complete(th_send_op* op, th_err err)
{
    op->err = err;
    th_op_set_flags(&op->base, TH_OP_COMPLETED);
    th_socket_post(op->socket, &op->base.base);
}

TH_LOCAL(th_err)
th_send_op_perform(th_send_op* op)
{
    th_op_clear_flags(&op->base, TH_OP_IMMEDIATE);
    size_t result = 0;
    th_err err = th_socket_send(op->socket, (const char*)op->addr + op->pos, op->len - op->pos, &result);
    if (err != TH_ERR_OK)
        return err;
    op->pos += result;
    if (op->pos == op->len)
        return TH_ERR_OK;
    return TH_ERR_SYSTEM(TH_EAGAIN);
}

TH_LOCAL(void)
th_send_op_fn(void* self)
{
    th_send_op* op = self;
    if (th_op_get_flags(&op->base) & TH_OP_COMPLETED) {
        th_send_op_finalize(op);
        return;
    }
    th_err err = th_send_op_perform(op);
    if (th_send_op_is_retryable(err)) {
        err = th_socket_submit(op->socket, &op->base);
        if (err == TH_ERR_OK)
            return;
    }
    th_send_op_complete(op, err);
}

TH_LOCAL(void)
th_send_op_abort(void* self, th_err err)
{
    th_send_op_complete(self, err);
}

TH_PRIVATE(void)
th_send_op_init(th_send_op* op, th_socket* socket, const void* addr, size_t len, th_send_cb callback, void* user_data)
{
    th_op_init(&op->base, TH_OP_WRITE, th_send_op_fn, th_send_op_abort);
    op->socket = socket;
    op->addr = addr;
    op->len = len;
    op->pos = 0;
    op->callback = callback;
    op->user_data = user_data;
    op->err = TH_ERR_OK;
}
