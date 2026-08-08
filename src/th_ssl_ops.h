#ifndef TH_SSL_OPS_H
#define TH_SSL_OPS_H

#include "th_config.h"

#if TH_WITH_SSL
#include <th.h>

#include <openssl/ssl.h>

/** th_ssl_ops
 * @brief The raw OpenSSL calls th_ssl_session/th_ssl_context perform.
 * Injected at construction time so tests can fake SSL without a real
 * SSL_CTX/SSL/BIO. Each call mirrors the underlying OpenSSL function
 * directly (same return value meaning), so callers can interpret
 * results (and call SSL_get_error on failure) themselves.
 */
typedef struct th_ssl_ops {
    /* SSL_CTX (th_ssl_context) */
    SSL_CTX* (*ctx_new)(void* self);
    void (*ctx_free)(void* self, SSL_CTX* ctx);
    int (*ctx_use_certificate_chain_file)(void* self, SSL_CTX* ctx, const char* cert);
    int (*ctx_use_private_key_file)(void* self, SSL_CTX* ctx, const char* key);
    int (*ctx_set_min_proto_version)(void* self, SSL_CTX* ctx);
    int (*ctx_set_cipher_list)(void* self, SSL_CTX* ctx, const char* ciphers);
    void (*ctx_set_session_cache_off)(void* self, SSL_CTX* ctx);

    /* SSL (th_ssl_session) */
    SSL* (*new_ssl)(void* self, SSL_CTX* ctx);
    void (*free_ssl)(void* self, SSL* ssl);
    void (*set_bio)(void* self, SSL* ssl, BIO* rbio, BIO* wbio);
    void (*set_accept_state)(void* self, SSL* ssl);
    void (*set_partial_write)(void* self, SSL* ssl);
    int (*do_handshake)(void* self, SSL* ssl);
    int (*read)(void* self, SSL* ssl, void* buf, int len);
    int (*write)(void* self, SSL* ssl, const void* buf, int len);
    int (*get_error)(void* self, SSL* ssl, int ret);
} th_ssl_ops;

TH_PRIVATE(th_ssl_ops*)
th_ssl_ops_os(void);

#endif
#endif
