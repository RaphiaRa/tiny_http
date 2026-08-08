#include "th_config.h"

#if TH_WITH_SSL

#include "th_ssl_context.h"
#include "th_ssl_ops.h"
#include "th_test.h"

static int th_fake_ctx_instance;
#define TH_FAKE_CTX ((SSL_CTX*)&th_fake_ctx_instance)

typedef struct th_fake_ssl_ops {
    th_ssl_ops base;
    bool ctx_new_fails;
    int use_certificate_chain_file_ret;
    int use_private_key_file_ret;
    int set_min_proto_version_ret;
    int set_cipher_list_ret;
    bool freed;
} th_fake_ssl_ops;

static SSL_CTX*
th_fake_ctx_new(void* self)
{
    th_fake_ssl_ops* ops = self;
    return ops->ctx_new_fails ? NULL : TH_FAKE_CTX;
}

static void
th_fake_ctx_free(void* self, SSL_CTX* ctx)
{
    (void)ctx;
    th_fake_ssl_ops* ops = self;
    ops->freed = true;
}

static int
th_fake_ctx_use_certificate_chain_file(void* self, SSL_CTX* ctx, const char* cert)
{
    (void)ctx;
    (void)cert;
    th_fake_ssl_ops* ops = self;
    return ops->use_certificate_chain_file_ret;
}

static int
th_fake_ctx_use_private_key_file(void* self, SSL_CTX* ctx, const char* key)
{
    (void)ctx;
    (void)key;
    th_fake_ssl_ops* ops = self;
    return ops->use_private_key_file_ret;
}

static int
th_fake_ctx_set_min_proto_version(void* self, SSL_CTX* ctx)
{
    (void)ctx;
    th_fake_ssl_ops* ops = self;
    return ops->set_min_proto_version_ret;
}

static int
th_fake_ctx_set_cipher_list(void* self, SSL_CTX* ctx, const char* ciphers)
{
    (void)ctx;
    (void)ciphers;
    th_fake_ssl_ops* ops = self;
    return ops->set_cipher_list_ret;
}

static void
th_fake_ctx_set_session_cache_off(void* self, SSL_CTX* ctx)
{
    (void)self;
    (void)ctx;
}

static void
th_fake_ssl_ops_init(th_fake_ssl_ops* ops)
{
    ops->base.ctx_new = th_fake_ctx_new;
    ops->base.ctx_free = th_fake_ctx_free;
    ops->base.ctx_use_certificate_chain_file = th_fake_ctx_use_certificate_chain_file;
    ops->base.ctx_use_private_key_file = th_fake_ctx_use_private_key_file;
    ops->base.ctx_set_min_proto_version = th_fake_ctx_set_min_proto_version;
    ops->base.ctx_set_cipher_list = th_fake_ctx_set_cipher_list;
    ops->base.ctx_set_session_cache_off = th_fake_ctx_set_session_cache_off;
    ops->ctx_new_fails = false;
    ops->use_certificate_chain_file_ret = 1;
    ops->use_private_key_file_ret = 1;
    ops->set_min_proto_version_ret = 1;
    ops->set_cipher_list_ret = 1;
    ops->freed = false;
}

TH_TEST_BEGIN(ssl_context)
{
    th_fake_ssl_ops ops;
    th_fake_ssl_ops_init(&ops);

    TH_TEST_CASE_BEGIN(ssl_context_init_succeeds_with_valid_key_and_cert)
    {
        th_ssl_context context;
        TH_EXPECT(th_ssl_context_init(&context, &ops.base, "key.pem", "cert.pem") == TH_ERR_OK);
        TH_EXPECT(context.ctx == TH_FAKE_CTX);

        th_ssl_context_deinit(&context);
        TH_EXPECT(ops.freed);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(ssl_context_init_fails_when_ctx_new_fails)
    {
        ops.ctx_new_fails = true;
        th_ssl_context context;
        TH_EXPECT(th_ssl_context_init(&context, &ops.base, "key.pem", "cert.pem") != TH_ERR_OK);
        TH_EXPECT(context.ctx == NULL);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(ssl_context_init_fails_and_frees_ctx_when_certificate_load_fails)
    {
        ops.use_certificate_chain_file_ret = 0;
        th_ssl_context context;
        TH_EXPECT(th_ssl_context_init(&context, &ops.base, "key.pem", "cert.pem") != TH_ERR_OK);
        TH_EXPECT(context.ctx == NULL);
        TH_EXPECT(ops.freed);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(ssl_context_init_fails_and_frees_ctx_when_private_key_load_fails)
    {
        ops.use_private_key_file_ret = 0;
        th_ssl_context context;
        TH_EXPECT(th_ssl_context_init(&context, &ops.base, "key.pem", "cert.pem") != TH_ERR_OK);
        TH_EXPECT(context.ctx == NULL);
        TH_EXPECT(ops.freed);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(ssl_context_init_fails_and_frees_ctx_when_min_proto_version_fails)
    {
        ops.set_min_proto_version_ret = 0;
        th_ssl_context context;
        TH_EXPECT(th_ssl_context_init(&context, &ops.base, "key.pem", "cert.pem") != TH_ERR_OK);
        TH_EXPECT(context.ctx == NULL);
        TH_EXPECT(ops.freed);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(ssl_context_init_fails_and_frees_ctx_when_cipher_list_fails)
    {
        ops.set_cipher_list_ret = 0;
        th_ssl_context context;
        TH_EXPECT(th_ssl_context_init(&context, &ops.base, "key.pem", "cert.pem") != TH_ERR_OK);
        TH_EXPECT(context.ctx == NULL);
        TH_EXPECT(ops.freed);
    }
    TH_TEST_CASE_END
}
TH_TEST_END

#endif
