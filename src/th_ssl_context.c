#include "th_ssl_context.h"

#if TH_WITH_SSL

#include "th_log.h"
#include "th_ssl_error.h"

#include <openssl/err.h>
#include <openssl/ssl.h>

#undef TH_LOG_TAG
#define TH_LOG_TAG "ssl_context"

TH_PRIVATE(th_err)
th_ssl_context_init(th_ssl_context* context, const char* key, const char* cert)
{
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();

    context->ctx = SSL_CTX_new(TLS_server_method());
    if (!context->ctx) {
        TH_LOG_FATAL("Failed to create SSL context");
        goto cleanup;
    }

    if (SSL_CTX_use_certificate_chain_file(context->ctx, cert) <= 0) {
        TH_LOG_FATAL("Failed to load certificate file");
        goto cleanup;
    }

    if (SSL_CTX_use_PrivateKey_file(context->ctx, key, SSL_FILETYPE_PEM) <= 0) {
        TH_LOG_FATAL("Failed to load private key file");
        goto cleanup;
    }

    if (!SSL_CTX_set_min_proto_version(context->ctx, TLS1_3_VERSION)) {
        TH_LOG_FATAL("Failed to set minimum protocol version");
        goto cleanup;
    }

    if (SSL_CTX_set_cipher_list(context->ctx, "MEDIUM:HIGH:!aNULL!MD5:!RC4!3DES") <= 0) {
        TH_LOG_FATAL("Failed to set cipher list");
        goto cleanup;
    }

    SSL_CTX_set_session_cache_mode(context->ctx, SSL_SESS_CACHE_OFF);
    context->smem_method = NULL;
    return TH_ERR_OK;
cleanup:
    if (context->ctx) {
        SSL_CTX_free(context->ctx);
        context->ctx = NULL;
    }
    return th_ssl_handle_error_stack();
}

TH_PRIVATE(void)
th_ssl_context_deinit(th_ssl_context* context)
{
    if (context->smem_method)
        BIO_meth_free(context->smem_method);
    if (context->ctx)
        SSL_CTX_free(context->ctx);
}
#endif
