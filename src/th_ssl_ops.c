#include "th_ssl_ops.h"

#if TH_WITH_SSL

#include "th_log.h"

#include <openssl/err.h>

#undef TH_LOG_TAG
#define TH_LOG_TAG "ssl"

/** th_ssl_ops_os_log_error_stack
 * @brief Drains and logs OpenSSL's per-thread error queue. Call right
 * after a real OpenSSL call reports failure — WANT_READ/WANT_WRITE never
 * push queue entries, so calling this unconditionally on ret<=0/NULL is
 * safe and simply logs nothing for those.
 */
TH_LOCAL(void)
th_ssl_ops_os_log_error_stack(void)
{
    unsigned long code;
    while ((code = ERR_get_error())) {
        TH_LOG_ERROR("%s", ERR_reason_error_string(code));
    }
}

TH_LOCAL(SSL_CTX*)
th_ssl_ops_os_ctx_new(void* self)
{
    (void)self;
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
    SSL_CTX* ctx = SSL_CTX_new(TLS_server_method());
    if (!ctx)
        th_ssl_ops_os_log_error_stack();
    return ctx;
}

TH_LOCAL(void)
th_ssl_ops_os_ctx_free(void* self, SSL_CTX* ctx)
{
    (void)self;
    SSL_CTX_free(ctx);
}

TH_LOCAL(int)
th_ssl_ops_os_ctx_use_certificate_chain_file(void* self, SSL_CTX* ctx, const char* cert)
{
    (void)self;
    int ret = SSL_CTX_use_certificate_chain_file(ctx, cert);
    if (ret <= 0)
        th_ssl_ops_os_log_error_stack();
    return ret;
}

TH_LOCAL(int)
th_ssl_ops_os_ctx_use_private_key_file(void* self, SSL_CTX* ctx, const char* key)
{
    (void)self;
    int ret = SSL_CTX_use_PrivateKey_file(ctx, key, SSL_FILETYPE_PEM);
    if (ret <= 0)
        th_ssl_ops_os_log_error_stack();
    return ret;
}

TH_LOCAL(int)
th_ssl_ops_os_ctx_set_min_proto_version(void* self, SSL_CTX* ctx)
{
    (void)self;
    int ret = SSL_CTX_set_min_proto_version(ctx, TLS1_3_VERSION) != 0;
    if (!ret)
        th_ssl_ops_os_log_error_stack();
    return ret;
}

TH_LOCAL(int)
th_ssl_ops_os_ctx_set_cipher_list(void* self, SSL_CTX* ctx, const char* ciphers)
{
    (void)self;
    int ret = SSL_CTX_set_cipher_list(ctx, ciphers);
    if (ret <= 0)
        th_ssl_ops_os_log_error_stack();
    return ret;
}

TH_LOCAL(void)
th_ssl_ops_os_ctx_set_session_cache_off(void* self, SSL_CTX* ctx)
{
    (void)self;
    SSL_CTX_set_session_cache_mode(ctx, SSL_SESS_CACHE_OFF);
}

TH_LOCAL(SSL*)
th_ssl_ops_os_new_ssl(void* self, SSL_CTX* ctx)
{
    (void)self;
    SSL* ssl = SSL_new(ctx);
    if (!ssl)
        th_ssl_ops_os_log_error_stack();
    return ssl;
}

TH_LOCAL(void)
th_ssl_ops_os_free_ssl(void* self, SSL* ssl)
{
    (void)self;
    SSL_free(ssl);
}

TH_LOCAL(void)
th_ssl_ops_os_set_bio(void* self, SSL* ssl, BIO* rbio, BIO* wbio)
{
    (void)self;
    SSL_set_bio(ssl, rbio, wbio);
}

TH_LOCAL(void)
th_ssl_ops_os_set_accept_state(void* self, SSL* ssl)
{
    (void)self;
    SSL_set_accept_state(ssl);
}

TH_LOCAL(void)
th_ssl_ops_os_set_partial_write(void* self, SSL* ssl)
{
    (void)self;
    SSL_set_mode(ssl, SSL_MODE_ENABLE_PARTIAL_WRITE);
}

TH_LOCAL(int)
th_ssl_ops_os_do_handshake(void* self, SSL* ssl)
{
    (void)self;
    int ret = SSL_do_handshake(ssl);
    if (ret <= 0)
        th_ssl_ops_os_log_error_stack();
    return ret;
}

TH_LOCAL(int)
th_ssl_ops_os_read(void* self, SSL* ssl, void* buf, int len)
{
    (void)self;
    int ret = SSL_read(ssl, buf, len);
    if (ret <= 0)
        th_ssl_ops_os_log_error_stack();
    return ret;
}

TH_LOCAL(int)
th_ssl_ops_os_write(void* self, SSL* ssl, const void* buf, int len)
{
    (void)self;
    int ret = SSL_write(ssl, buf, len);
    if (ret <= 0)
        th_ssl_ops_os_log_error_stack();
    return ret;
}

TH_LOCAL(int)
th_ssl_ops_os_get_error(void* self, SSL* ssl, int ret)
{
    (void)self;
    return SSL_get_error(ssl, ret);
}

TH_PRIVATE(th_ssl_ops*)
th_ssl_ops_os(void)
{
    static th_ssl_ops ops = {
        .ctx_new = th_ssl_ops_os_ctx_new,
        .ctx_free = th_ssl_ops_os_ctx_free,
        .ctx_use_certificate_chain_file = th_ssl_ops_os_ctx_use_certificate_chain_file,
        .ctx_use_private_key_file = th_ssl_ops_os_ctx_use_private_key_file,
        .ctx_set_min_proto_version = th_ssl_ops_os_ctx_set_min_proto_version,
        .ctx_set_cipher_list = th_ssl_ops_os_ctx_set_cipher_list,
        .ctx_set_session_cache_off = th_ssl_ops_os_ctx_set_session_cache_off,
        .new_ssl = th_ssl_ops_os_new_ssl,
        .free_ssl = th_ssl_ops_os_free_ssl,
        .set_bio = th_ssl_ops_os_set_bio,
        .set_accept_state = th_ssl_ops_os_set_accept_state,
        .set_partial_write = th_ssl_ops_os_set_partial_write,
        .do_handshake = th_ssl_ops_os_do_handshake,
        .read = th_ssl_ops_os_read,
        .write = th_ssl_ops_os_write,
        .get_error = th_ssl_ops_os_get_error,
    };
    return &ops;
}

#endif
