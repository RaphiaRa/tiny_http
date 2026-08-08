#ifndef TH_SSL_SEND_H
#define TH_SSL_SEND_H

#include "th_config.h"

#if TH_WITH_SSL
#include <th.h>

#include "th_file.h"
#include "th_iov.h"
#include "th_send.h"
#include "th_socket.h"
#include "th_ssl_io.h"
#include "th_ssl_session.h"

#define TH_SSL_SEND_CHUNK_LEN (16 * 1024)

/** th_ssl_send_op
 * @brief Writes iov (mutated in place as buffers are consumed) as
 * plaintext through a th_ssl_session (shuttling ciphertext over socket
 * as needed), followed by len bytes of file starting at offset if file
 * is non-NULL, retrying in TH_SSL_SEND_CHUNK_LEN-sized steps until every
 * byte has been written or an error occurs. After init, the first
 * th_ssl_io_op write is already in flight.
 */
typedef struct th_ssl_send_op {
    th_ssl_io_op io;
    th_socket* socket;
    th_ssl_session* session;
    th_send_cb callback;
    void* user_data;
    th_iov* iov;
    size_t iovcnt;
    th_file* file;
    size_t offset;
    size_t len;
    size_t file_pos;
    size_t pos;
    char buffer[TH_SSL_SEND_CHUNK_LEN];
} th_ssl_send_op;

TH_PRIVATE(void)
th_ssl_send_op_init(th_ssl_send_op* op, th_socket* socket, th_ssl_session* session,
                    th_iov* iov, size_t iovcnt, th_file* file, size_t offset, size_t len,
                    th_send_cb callback, void* user_data);

#endif
#endif
