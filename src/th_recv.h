#ifndef TH_RECV_H
#define TH_RECV_H

#include <th.h>

#include "th_op.h"
#include "th_socket.h"

#include <stdbool.h>

typedef void (*th_recv_cb)(void* user_data, size_t size, th_err err);

/** th_recv_op
 * @brief Reads from a th_socket into addr. If exact is false, completes
 * as soon as any bytes arrive (0 bytes => TH_ERR_EOF); if true, retries
 * until exactly len bytes have been read or an error/EOF occurs. After
 * init, start with th_op_perform(&op->base): it performs the first,
 * immediate recv attempt and submits to the socket for readiness only
 * on TH_EAGAIN/TH_EWOULDBLOCK. On completion the op posts itself to the
 * socket's loop and callback runs from that later drain, never the
 * caller's stack.
 */
typedef struct th_recv_op {
    th_op base;
    th_socket* socket;
    th_recv_cb callback;
    void* user_data;
    void* addr;
    size_t len;
    size_t pos;
    bool exact;
    th_err err;
} th_recv_op;

TH_PRIVATE(void)
th_recv_op_init(th_recv_op* op, th_socket* socket, void* addr, size_t len, bool exact, th_recv_cb callback, void* user_data);

#endif
