#include "th_ssl_io.h"

#if TH_WITH_SSL

#include "th_system_error.h"
#include "th_utility.h"

TH_LOCAL(bool)
th_ssl_io_op_is_retryable(th_err err)
{
    return err == TH_ERR_SYSTEM(TH_EAGAIN)
           || err == TH_ERR_SYSTEM(TH_EWOULDBLOCK);
}

TH_LOCAL(void)
th_ssl_io_op_finalize(th_ssl_io_op* op)
{
    op->callback(op->user_data, op->result, op->err);
}

TH_LOCAL(void)
th_ssl_io_op_complete(th_ssl_io_op* op, size_t result, th_err err)
{
    op->result = result;
    op->err = err;
    th_op_set_flags(&op->base, TH_OP_COMPLETED);
    th_socket_post(op->socket, &op->base.base);
}

/** th_ssl_io_op_step
 * @brief Calls the session step for op->kind once. Returns TH_ERR_OK when
 * done, TH_ERR_SYSTEM(TH_EAGAIN) when a raw ciphertext shuttle (recv or
 * send on op->socket) must run before retrying — op->shuttling_write says
 * which direction — or any other th_err on failure.
 */
TH_LOCAL(th_err)
th_ssl_io_op_step(th_ssl_io_op* op)
{
    th_err err = TH_ERR_OK;
    th_ssl_result result;
    switch (op->kind) {
    case TH_SSL_IO_HANDSHAKE:
        result = th_ssl_session_handshake(op->session, &err);
        break;
    case TH_SSL_IO_READ: {
        size_t out = 0;
        result = th_ssl_session_read(op->session, op->buf, op->len, &out, &err);
        op->result = out;
        break;
    }
    case TH_SSL_IO_WRITE: {
        size_t out = 0;
        result = th_ssl_session_write(op->session, op->buf, op->len, &out, &err);
        op->result = out;
        break;
    }
    default:
        TH_ASSERT(0 && "Invalid th_ssl_io_kind");
        return TH_ERR_SSL(0);
    }
    switch (result) {
    case TH_SSL_DONE:
        return TH_ERR_OK;
    case TH_SSL_WANT_READ:
        op->shuttling_write = false;
        return TH_ERR_SYSTEM(TH_EAGAIN);
    case TH_SSL_WANT_WRITE:
        op->shuttling_write = true;
        return TH_ERR_SYSTEM(TH_EAGAIN);
    default:
        return err;
    }
}

/** th_ssl_io_op_shuttle
 * @brief Drains pending ciphertext to op->socket (shuttling_write) or
 * reads more ciphertext in from it, once. Returns TH_ERR_OK when that
 * raw transfer completed (retry the session step next), or propagates
 * TH_EAGAIN/an error from the raw socket call.
 */
TH_LOCAL(th_err)
th_ssl_io_op_shuttle(th_ssl_io_op* op)
{
    th_iov iov;
    size_t result = 0;
    th_err err;
    if (op->shuttling_write) {
        th_ssl_session_get_ciphertext_out(op->session, &iov);
        if (iov.len == 0)
            return TH_ERR_OK;
        err = th_socket_sendvec(op->socket, &iov, 1, &result);
        if (err != TH_ERR_OK)
            return err;
        th_ssl_session_consume_ciphertext_out(op->session, result);
        return TH_ERR_OK;
    }
    th_ssl_session_get_ciphertext_in_buf(op->session, &iov);
    err = th_socket_recv(op->socket, iov.base, iov.len, &result);
    if (err != TH_ERR_OK)
        return err;
    th_ssl_session_fed_ciphertext_in(op->session, result);
    return TH_ERR_OK;
}

/** th_ssl_io_op_perform
 * @brief Alternates session steps with raw ciphertext shuttles until the
 * step is done/errors, or (for READ/WRITE) has made plaintext progress
 * and its last-requested shuttle has drained/fed — matching TCP recv/send
 * semantics where a short transfer is a valid completion, not something
 * to retry into the same buffer. op->draining marks that plaintext
 * progress already happened and only the shuttle remains, so a step that
 * gets interrupted by EAGAIN mid-shuttle resumes straight into the
 * shuttle on the next call instead of re-invoking SSL_read/SSL_write and
 * overwriting op->result.
 */
TH_LOCAL(th_err)
th_ssl_io_op_perform(th_ssl_io_op* op)
{
    th_op_clear_flags(&op->base, TH_OP_IMMEDIATE);
    for (;;) {
        if (!op->draining) {
            th_err err = th_ssl_io_op_step(op);
            if (err != TH_ERR_SYSTEM(TH_EAGAIN))
                return err;
            if (op->kind != TH_SSL_IO_HANDSHAKE && op->result > 0)
                op->draining = true;
        }
        th_err err = th_ssl_io_op_shuttle(op);
        if (err != TH_ERR_OK)
            return err;
        if (op->draining)
            return TH_ERR_OK;
    }
}

TH_LOCAL(void)
th_ssl_io_op_fn(void* self)
{
    th_ssl_io_op* op = self;
    if (th_op_get_flags(&op->base) & TH_OP_COMPLETED) {
        th_ssl_io_op_finalize(op);
        return;
    }
    op->base.type = op->shuttling_write ? TH_OP_WRITE : TH_OP_READ;
    th_err err = th_ssl_io_op_perform(op);
    if (th_ssl_io_op_is_retryable(err)) {
        op->base.type = op->shuttling_write ? TH_OP_WRITE : TH_OP_READ;
        err = th_socket_submit(op->socket, &op->base);
        if (err == TH_ERR_OK)
            return;
    }
    th_ssl_io_op_complete(op, op->result, err);
}

TH_LOCAL(void)
th_ssl_io_op_abort(void* self, th_err err)
{
    th_ssl_io_op_complete(self, 0, err);
}

TH_LOCAL(void)
th_ssl_io_op_init(th_ssl_io_op* op, th_socket* socket, th_ssl_session* session, th_ssl_io_kind kind, th_ssl_io_cb callback, void* user_data)
{
    th_op_init(&op->base, TH_OP_READ, th_ssl_io_op_fn, th_ssl_io_op_abort);
    op->socket = socket;
    op->session = session;
    op->kind = kind;
    op->buf = NULL;
    op->len = 0;
    op->result = 0;
    op->shuttling_write = false;
    op->draining = false;
    op->callback = callback;
    op->user_data = user_data;
    op->err = TH_ERR_OK;
}

TH_PRIVATE(void)
th_ssl_io_op_init_handshake(th_ssl_io_op* op, th_socket* socket, th_ssl_session* session, th_ssl_io_cb callback, void* user_data)
{
    th_ssl_io_op_init(op, socket, session, TH_SSL_IO_HANDSHAKE, callback, user_data);
}

TH_PRIVATE(void)
th_ssl_io_op_init_read(th_ssl_io_op* op, th_socket* socket, th_ssl_session* session, void* buf, size_t len, th_ssl_io_cb callback, void* user_data)
{
    th_ssl_io_op_init(op, socket, session, TH_SSL_IO_READ, callback, user_data);
    op->buf = buf;
    op->len = len;
}

TH_PRIVATE(void)
th_ssl_io_op_init_write(th_ssl_io_op* op, th_socket* socket, th_ssl_session* session, const void* buf, size_t len, th_ssl_io_cb callback, void* user_data)
{
    th_ssl_io_op_init(op, socket, session, TH_SSL_IO_WRITE, callback, user_data);
    op->buf = (void*)buf;
    op->len = len;
}

#endif
