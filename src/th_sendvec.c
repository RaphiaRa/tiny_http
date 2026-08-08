#include "th_sendvec.h"
#include "th_system_error.h"

TH_LOCAL(bool)
th_sendvec_op_is_retryable(th_err err)
{
    return err == TH_ERR_SYSTEM(TH_EAGAIN)
           || err == TH_ERR_SYSTEM(TH_EWOULDBLOCK);
}

TH_LOCAL(void)
th_sendvec_op_finalize(th_sendvec_op* op)
{
    op->callback(op->user_data, op->pos, op->err);
}

TH_LOCAL(void)
th_sendvec_op_complete(th_sendvec_op* op, th_err err)
{
    op->err = err;
    th_op_set_flags(&op->base, TH_OP_COMPLETED);
    th_socket_post(op->socket, &op->base.base);
}

TH_LOCAL(th_err)
th_sendvec_op_perform(th_sendvec_op* op)
{
    th_op_clear_flags(&op->base, TH_OP_IMMEDIATE);
    size_t result = 0;
    th_err err = th_socket_sendvec(op->socket, op->iov, op->iovcnt, &result);
    if (err != TH_ERR_OK)
        return err;
    op->pos += result;
    th_iov_consume(&op->iov, &op->iovcnt, result);
    if (op->iovcnt == 0)
        return TH_ERR_OK;
    return TH_ERR_SYSTEM(TH_EAGAIN);
}

TH_LOCAL(void)
th_sendvec_op_fn(void* self)
{
    th_sendvec_op* op = self;
    if (th_op_get_flags(&op->base) & TH_OP_COMPLETED) {
        th_sendvec_op_finalize(op);
        return;
    }
    th_err err = th_sendvec_op_perform(op);
    if (th_sendvec_op_is_retryable(err)) {
        err = th_socket_submit(op->socket, &op->base);
        if (err == TH_ERR_OK)
            return;
    }
    th_sendvec_op_complete(op, err);
}

TH_LOCAL(void)
th_sendvec_op_abort(void* self, th_err err)
{
    th_sendvec_op_complete(self, err);
}

TH_PRIVATE(void)
th_sendvec_op_init(th_sendvec_op* op, th_socket* socket, th_iov* iov, size_t iovcnt, th_send_cb callback, void* user_data)
{
    th_op_init(&op->base, TH_OP_WRITE, th_sendvec_op_fn, th_sendvec_op_abort);
    op->socket = socket;
    op->iov = iov;
    op->iovcnt = iovcnt;
    op->pos = 0;
    op->callback = callback;
    op->user_data = user_data;
    op->err = TH_ERR_OK;
}
