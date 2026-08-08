#include "th_ssl_send.h"

#if TH_WITH_SSL

#include "th_utility.h"

#include <string.h>

TH_LOCAL(void)
th_ssl_send_op_finalize(th_ssl_send_op* op, th_err err)
{
    op->callback(op->user_data, op->pos, err);
}

TH_LOCAL(void)
th_ssl_send_op_start(th_ssl_send_op* op);

TH_LOCAL(void)
th_ssl_send_op_io_complete(void* user_data, size_t size, th_err err)
{
    th_ssl_send_op* op = user_data;
    if (err != TH_ERR_OK) {
        th_ssl_send_op_finalize(op, err);
        return;
    }
    op->pos += size;
    th_ssl_send_op_start(op);
}

/** th_ssl_send_op_fill
 * @brief Consumes iov (and then file, if the header didn't fill the
 * chunk) into op->buffer. Returns the number of bytes filled.
 */
TH_LOCAL(th_err)
th_ssl_send_op_fill(th_ssl_send_op* op, size_t* out)
{
    size_t bufpos = 0;
    while (op->iovcnt > 0 && bufpos < TH_SSL_SEND_CHUNK_LEN) {
        size_t avail = TH_SSL_SEND_CHUNK_LEN - bufpos;
        size_t to_copy = TH_MIN(avail, op->iov[0].len);
        memcpy(op->buffer + bufpos, op->iov[0].base, to_copy);
        bufpos += to_copy;
        th_iov_consume(&op->iov, &op->iovcnt, to_copy);
    }
    if (op->file && bufpos < TH_SSL_SEND_CHUNK_LEN) {
        size_t remaining = op->len - op->file_pos;
        size_t readlen = TH_MIN(TH_SSL_SEND_CHUNK_LEN - bufpos, remaining);
        if (readlen > 0) {
            size_t bytes_read = 0;
            th_err err = th_file_read(op->file, op->buffer + bufpos, readlen, op->offset + op->file_pos, &bytes_read);
            if (err != TH_ERR_OK && bufpos == 0)
                return err;
            op->file_pos += bytes_read;
            bufpos += bytes_read;
        }
    }
    *out = bufpos;
    return TH_ERR_OK;
}

TH_LOCAL(void)
th_ssl_send_op_start(th_ssl_send_op* op)
{
    size_t chunk_len = 0;
    th_err err = th_ssl_send_op_fill(op, &chunk_len);
    if (err != TH_ERR_OK) {
        th_ssl_send_op_finalize(op, err);
        return;
    }
    if (chunk_len == 0) {
        th_ssl_send_op_finalize(op, TH_ERR_OK);
        return;
    }
    th_ssl_io_op_init_write(&op->io, op->socket, op->session, op->buffer, chunk_len, th_ssl_send_op_io_complete, op);
    th_op_perform(&op->io.base);
}

TH_PRIVATE(void)
th_ssl_send_op_init(th_ssl_send_op* op, th_socket* socket, th_ssl_session* session,
                    th_iov* iov, size_t iovcnt, th_file* file, size_t offset, size_t len,
                    th_send_cb callback, void* user_data)
{
    op->socket = socket;
    op->session = session;
    op->iov = iov;
    op->iovcnt = iovcnt;
    op->file = file;
    op->offset = offset;
    op->len = len;
    op->file_pos = 0;
    op->pos = 0;
    op->callback = callback;
    op->user_data = user_data;
    th_ssl_send_op_start(op);
}

#endif
