#ifndef TH_SENDFILE_H
#define TH_SENDFILE_H

#include <th.h>

#include "th_iov.h"
#include "th_op.h"
#include "th_send.h"
#include "th_socket.h"

/** th_sendfile_op
 * @brief Sends header (iov/iovcnt, may be empty) followed by len bytes of
 * file starting at offset, retrying in TH_CONFIG_SENDFILE_CHUNK_LEN-sized
 * steps until every byte (header + file) has been written or an error
 * occurs. iov is mutated in place as header buffers are consumed. After
 * init, start with th_op_perform(&op->base). On completion the op posts
 * itself to the socket's loop and callback runs from that later drain,
 * never the caller's stack.
 */
typedef struct th_sendfile_op {
    th_op base;
    th_socket* socket;
    th_send_cb callback;
    void* user_data;
    th_iov* iov;
    size_t iovcnt;
    th_file* file;
    size_t offset;
    size_t len;
    size_t header_len;
    size_t pos;
    th_err err;
} th_sendfile_op;

TH_PRIVATE(void)
th_sendfile_op_init(th_sendfile_op* op, th_socket* socket, th_iov* iov, size_t iovcnt, th_file* file, size_t offset, size_t len, th_send_cb callback, void* user_data);

#endif
