#ifndef TH_ACCEPT_H
#define TH_ACCEPT_H

#include <th.h>

#include "th_acceptor.h"
#include "th_address.h"
#include "th_op.h"

typedef void (*th_accept_cb)(void* user_data, int fd, th_err err);

/** th_accept_op
 * @brief Accepts one connection on a th_acceptor. After init, start with
 * th_op_perform(&op->base): it performs the first, immediate accept
 * attempt and submits to the acceptor for readiness only on
 * TH_EAGAIN/TH_EWOULDBLOCK. On completion the op posts itself to the
 * acceptor's loop and callback runs from that later drain, never the
 * caller's stack.
 */
typedef struct th_accept_op {
    th_op base;
    th_acceptor* acceptor;
    th_address* addr;
    th_accept_cb callback;
    void* user_data;
    int fd;
    th_err err;
} th_accept_op;

TH_PRIVATE(void)
th_accept_op_init(th_accept_op* op, th_acceptor* acceptor, th_address* addr, th_accept_cb callback, void* user_data);

#endif
