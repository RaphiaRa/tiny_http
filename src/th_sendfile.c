#include "th_sendfile.h"
#include "th_system_error.h"
#include "th_utility.h"

TH_LOCAL(bool)
th_sendfile_op_is_retryable(th_err err)
{
    return err == TH_ERR_SYSTEM(TH_EAGAIN)
        || err == TH_ERR_SYSTEM(TH_EWOULDBLOCK);
}

TH_LOCAL(void)
th_sendfile_op_finalize(th_sendfile_op* op)
{
    op->callback(op->user_data, op->pos, op->err);
}

TH_LOCAL(void)
th_sendfile_op_complete(th_sendfile_op* op, th_err err)
{
    op->err = err;
    th_op_set_flags(&op->base, TH_OP_COMPLETED);
    th_socket_post(op->socket, &op->base.base);
}

TH_LOCAL(th_err)
th_sendfile_op_perform(th_sendfile_op* op)
{
    size_t file_pos = op->pos > op->header_len ? op->pos - op->header_len : 0;
    size_t remaining = op->len - file_pos;
    size_t chunk = TH_MIN(remaining, TH_CONFIG_SENDFILE_CHUNK_LEN);

    size_t result = 0;
    th_err err = th_socket_sendfile(op->socket, op->iov, op->iovcnt, op->file, op->offset + file_pos, chunk, &result);
    if (err != TH_ERR_OK)
        return err;

    op->pos += result;
    th_iov_consume(&op->iov, &op->iovcnt, result);
    if (op->pos == op->header_len + op->len)
        return TH_ERR_OK;
    return TH_ERR_SYSTEM(TH_EAGAIN);
}

TH_LOCAL(void)
th_sendfile_op_fn(void* self)
{
    th_sendfile_op* op = self;
    if (th_op_get_flags(&op->base) & TH_OP_COMPLETED) {
        th_sendfile_op_finalize(op);
        return;
    }
    th_err err = th_sendfile_op_perform(op);
    if (th_sendfile_op_is_retryable(err)) {
        err = th_socket_submit(op->socket, &op->base);
        if (err == TH_ERR_OK)
            return;
    }
    th_sendfile_op_complete(op, err);
}

TH_LOCAL(void)
th_sendfile_op_abort(void* self, th_err err)
{
    th_sendfile_op_complete(self, err);
}

TH_PRIVATE(void)
th_sendfile_op_init(th_sendfile_op* op, th_socket* socket, th_iov* iov, size_t iovcnt, th_file* file, size_t offset, size_t len, th_send_cb callback, void* user_data)
{
    th_op_init(&op->base, TH_OP_WRITE, th_sendfile_op_fn, NULL, th_sendfile_op_abort);
    op->socket = socket;
    op->iov = iov;
    op->iovcnt = iovcnt;
    op->file = file;
    op->offset = offset;
    op->len = len;
    op->header_len = th_iov_bytes(iov, iovcnt);
    op->pos = 0;
    op->callback = callback;
    op->user_data = user_data;
    op->err = TH_ERR_OK;
}
