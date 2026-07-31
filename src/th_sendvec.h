#ifndef TH_SENDVEC_H
#define TH_SENDVEC_H

#include <th.h>

#include "th_iov.h"
#include "th_op.h"
#include "th_send.h"
#include "th_socket.h"

/** th_sendvec_op
 * @brief Writes an iovec to a th_socket, retrying until every byte
 * across all buffers has been written or an error occurs. iov is
 * mutated in place as buffers are consumed. After init, start with
 * th_op_perform(&op->base). On completion the op posts itself to the
 * socket's loop and callback runs from that later drain, never the
 * caller's stack.
 */
typedef struct th_sendvec_op {
    th_op base;
    th_socket* socket;
    th_send_cb callback;
    void* user_data;
    th_iov* iov;
    size_t iovcnt;
    size_t pos;
    th_err err;
} th_sendvec_op;

TH_PRIVATE(void)
th_sendvec_op_init(th_sendvec_op* op, th_socket* socket, th_iov* iov, size_t iovcnt, th_send_cb callback, void* user_data);

#endif
