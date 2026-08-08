#ifndef TH_SSL_IO_H
#define TH_SSL_IO_H

#include "th_config.h"

#if TH_WITH_SSL
#include <th.h>

#include "th_op.h"
#include "th_socket.h"
#include "th_ssl_session.h"

typedef void (*th_ssl_io_cb)(void* user_data, size_t size, th_err err);

typedef enum th_ssl_io_kind {
    TH_SSL_IO_HANDSHAKE,
    TH_SSL_IO_READ,
    TH_SSL_IO_WRITE,
} th_ssl_io_kind;

/** th_ssl_io_op
 * @brief Drives one th_ssl_session step (handshake/read/write) to
 * completion, shuttling ciphertext to/from socket in between as the
 * session reports TH_SSL_WANT_READ/TH_SSL_WANT_WRITE. After init, start
 * with th_op_perform(&op->base). On completion the op posts itself to
 * the socket's loop and callback runs from that later drain, never the
 * caller's stack.
 */
typedef struct th_ssl_io_op {
    th_op base;
    th_socket* socket;
    th_ssl_session* session;
    th_ssl_io_kind kind;
    void* buf; /* plaintext in (READ) / plaintext out (WRITE), unused for HANDSHAKE */
    size_t len;
    size_t result;
    bool shuttling_write; /* mid raw-socket-send draining ciphertext out */
    bool draining;        /* plaintext progress made; finish the shuttle, don't step again */
    th_ssl_io_cb callback;
    void* user_data;
    th_err err;
} th_ssl_io_op;

TH_PRIVATE(void)
th_ssl_io_op_init_handshake(th_ssl_io_op* op, th_socket* socket, th_ssl_session* session, th_ssl_io_cb callback, void* user_data);

TH_PRIVATE(void)
th_ssl_io_op_init_read(th_ssl_io_op* op, th_socket* socket, th_ssl_session* session, void* buf, size_t len, th_ssl_io_cb callback, void* user_data);

TH_PRIVATE(void)
th_ssl_io_op_init_write(th_ssl_io_op* op, th_socket* socket, th_ssl_session* session, const void* buf, size_t len, th_ssl_io_cb callback, void* user_data);

#endif
#endif
