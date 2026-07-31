#ifndef TH_SEND_H
#define TH_SEND_H

#include <th.h>

#include "th_op.h"
#include "th_socket.h"

#include <stdbool.h>

typedef void (*th_send_cb)(void* user_data, size_t size, th_err err);

/** th_send_op
 * @brief Writes addr to a th_socket. If exact is false, completes as
 * soon as any bytes are accepted; if true, retries until exactly len
 * bytes have been written or an error occurs. After init, start with
 * th_op_perform(&op->base). On completion the op posts itself to the
 * socket's loop and callback runs from that later drain, never the
 * caller's stack.
 */
typedef struct th_send_op {
    th_op base;
    th_socket* socket;
    th_send_cb callback;
    void* user_data;
    const void* addr;
    size_t len;
    size_t pos;
    bool exact;
    th_err err;
} th_send_op;

TH_PRIVATE(void)
th_send_op_init(th_send_op* op, th_socket* socket, const void* addr, size_t len, bool exact, th_send_cb callback, void* user_data);

#endif
