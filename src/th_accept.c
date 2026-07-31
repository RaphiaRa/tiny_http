#include "th_accept.h"
#include "th_system_error.h"

TH_LOCAL(bool)
th_accept_op_is_retryable(th_err err)
{
    return err == TH_ERR_SYSTEM(TH_EAGAIN)
           || err == TH_ERR_SYSTEM(TH_EWOULDBLOCK);
}

TH_LOCAL(void)
th_accept_op_finalize(th_accept_op* op)
{
    op->callback(op->user_data, op->fd, op->err);
}

TH_LOCAL(void)
th_accept_op_complete(th_accept_op* op, th_err err)
{
    op->err = err;
    th_op_set_flags(&op->base, TH_OP_COMPLETED);
    th_acceptor_post(op->acceptor, &op->base.base);
}

TH_LOCAL(th_err)
th_accept_op_perform(th_accept_op* op)
{
    th_op_clear_flags(&op->base, TH_OP_IMMEDIATE);
    th_address_init(op->addr);
    return th_acceptor_accept(op->acceptor, op->addr, &op->fd);
}

TH_LOCAL(void)
th_accept_op_fn(void* self)
{
    th_accept_op* op = self;
    if (th_op_get_flags(&op->base) & TH_OP_COMPLETED) {
        th_accept_op_finalize(op);
        return;
    }
    th_err err = th_accept_op_perform(op);
    if (th_accept_op_is_retryable(err)) {
        err = th_acceptor_submit(op->acceptor, &op->base);
        if (err == TH_ERR_OK)
            return;
    }
    th_accept_op_complete(op, err);
}

TH_LOCAL(void)
th_accept_op_abort(void* self, th_err err)
{
    th_accept_op_complete(self, err);
}

TH_PRIVATE(void)
th_accept_op_init(th_accept_op* op, th_acceptor* acceptor, th_address* addr, th_accept_cb callback, void* user_data)
{
    th_op_init(&op->base, TH_OP_READ, th_accept_op_fn, NULL, th_accept_op_abort);
    op->acceptor = acceptor;
    op->addr = addr;
    op->callback = callback;
    op->user_data = user_data;
    op->fd = -1;
    op->err = TH_ERR_OK;
}
