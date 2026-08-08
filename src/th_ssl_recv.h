#ifndef TH_SSL_RECV_H
#define TH_SSL_RECV_H

#include "th_config.h"

#if TH_WITH_SSL
#include <th.h>

#include "th_recv.h"
#include "th_socket.h"
#include "th_ssl_io.h"
#include "th_ssl_session.h"

#include <stdbool.h>

/** th_ssl_recv_op
 * @brief Reads plaintext from a th_ssl_session (shuttling ciphertext over
 * socket as needed) into addr. If exact is false, completes as soon as
 * any bytes arrive (0 bytes => TH_ERR_EOF); if true, retries until
 * exactly len bytes have been read or an error/EOF occurs. After init,
 * the first th_ssl_io_op read is already in flight (no separate perform
 * call needed, unlike th_recv_op).
 */
typedef struct th_ssl_recv_op {
    th_ssl_io_op io;
    th_socket* socket;
    th_ssl_session* session;
    th_recv_cb callback;
    void* user_data;
    void* addr;
    size_t len;
    size_t pos;
    bool exact;
} th_ssl_recv_op;

TH_PRIVATE(void)
th_ssl_recv_op_init(th_ssl_recv_op* op, th_socket* socket, th_ssl_session* session, void* addr, size_t len, bool exact, th_recv_cb callback, void* user_data);

#endif
#endif
