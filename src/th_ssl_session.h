#ifndef TH_SSL_SESSION_H
#define TH_SSL_SESSION_H

#include "th_config.h"

#if TH_WITH_SSL
#include <th.h>

#include "th_allocator.h"
#include "th_iov.h"
#include "th_ssl_context.h"
#include "th_ssl_ops.h"

#include <openssl/ssl.h>

/** th_ssl_result
 * @brief Outcome of one th_ssl_session step. TH_SSL_WANT_READ means more
 * ciphertext must be fed in (via fed_ciphertext_in) before retrying;
 * TH_SSL_WANT_WRITE means pending ciphertext (get_ciphertext_out) must be
 * drained before retrying.
 */
typedef enum th_ssl_result {
    TH_SSL_DONE,
    TH_SSL_WANT_READ,
    TH_SSL_WANT_WRITE,
    TH_SSL_ERROR,
} th_ssl_result;

/** th_ssl_session
 * @brief Drives an OpenSSL handshake/read/write over a pair of memory
 * BIOs. Has no knowledge of th_conn/th_socket/the reactor — purely
 * plaintext in/out on one side, ciphertext in/out on the other; the
 * caller is responsible for shuttling ciphertext to/from a real socket.
 */
typedef struct th_ssl_session {
    SSL* ssl;
    BIO* rbio;
    BIO* wbio;
    th_ssl_ops* ops;
} th_ssl_session;

TH_PRIVATE(th_err)
th_ssl_session_init(th_ssl_session* session, th_ssl_context* context, th_ssl_ops* ops, th_allocator* allocator);

TH_PRIVATE(void)
th_ssl_session_deinit(th_ssl_session* session);

TH_PRIVATE(th_ssl_result)
th_ssl_session_handshake(th_ssl_session* session, th_err* err);

TH_PRIVATE(th_ssl_result)
th_ssl_session_read(th_ssl_session* session, void* buf, size_t len, size_t* out, th_err* err);

TH_PRIVATE(th_ssl_result)
th_ssl_session_write(th_ssl_session* session, const void* buf, size_t len, size_t* out, th_err* err);

/** th_ssl_session_get_ciphertext_out
 * @brief Ciphertext produced by the last handshake/read/write step that
 * still needs to be sent over the real socket.
 */
TH_PRIVATE(void)
th_ssl_session_get_ciphertext_out(th_ssl_session* session, th_iov* iov);

TH_PRIVATE(void)
th_ssl_session_consume_ciphertext_out(th_ssl_session* session, size_t n);

/** th_ssl_session_get_ciphertext_in_buf
 * @brief Spare capacity to recv() real-socket ciphertext into.
 */
TH_PRIVATE(void)
th_ssl_session_get_ciphertext_in_buf(th_ssl_session* session, th_iov* iov);

TH_PRIVATE(void)
th_ssl_session_fed_ciphertext_in(th_ssl_session* session, size_t n);

#endif
#endif
