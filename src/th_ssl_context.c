#include "th_ssl_context.h"

#if TH_WITH_SSL

#include "th_log.h"
#include "th_ssl_ops.h"

#undef TH_LOG_TAG
#define TH_LOG_TAG "ssl_context"

TH_PRIVATE(th_err)
th_ssl_context_init(th_ssl_context* context, th_ssl_ops* ops, const char* key, const char* cert)
{
    context->ops = ops;
    context->smem_method = NULL;

    context->ctx = ops->ctx_new(ops);
    if (!context->ctx) {
        TH_LOG_FATAL("Failed to create SSL context");
        goto cleanup;
    }

    if (ops->ctx_use_certificate_chain_file(ops, context->ctx, cert) <= 0) {
        TH_LOG_FATAL("Failed to load certificate file");
        goto cleanup;
    }

    if (ops->ctx_use_private_key_file(ops, context->ctx, key) <= 0) {
        TH_LOG_FATAL("Failed to load private key file");
        goto cleanup;
    }

    if (!ops->ctx_set_min_proto_version(ops, context->ctx)) {
        TH_LOG_FATAL("Failed to set minimum protocol version");
        goto cleanup;
    }

    if (ops->ctx_set_cipher_list(ops, context->ctx, "MEDIUM:HIGH:!aNULL!MD5:!RC4!3DES") <= 0) {
        TH_LOG_FATAL("Failed to set cipher list");
        goto cleanup;
    }

    ops->ctx_set_session_cache_off(ops, context->ctx);
    return TH_ERR_OK;
cleanup:
    if (context->ctx) {
        ops->ctx_free(ops, context->ctx);
        context->ctx = NULL;
    }
    return TH_ERR_SSL(SSL_ERROR_SSL);
}

TH_PRIVATE(void)
th_ssl_context_deinit(th_ssl_context* context)
{
    if (context->smem_method)
        BIO_meth_free(context->smem_method);
    if (context->ctx)
        context->ops->ctx_free(context->ops, context->ctx);
}
#endif
