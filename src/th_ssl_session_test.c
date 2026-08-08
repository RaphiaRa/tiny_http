#include "th_config.h"

#if TH_WITH_SSL

#include "th_ssl_session.h"
#include "th_test.h"

#include <string.h>

/* Not a real OpenSSL SSL* — th_ssl_ops is fully faked below, so this
 * pointer is never dereferenced by real OpenSSL code, only handed back
 * into the fake ops. */
static int th_fake_ssl_instance;
#define TH_FAKE_SSL ((SSL*)&th_fake_ssl_instance)

typedef enum th_fake_ssl_call {
    TH_FAKE_SSL_CALL_NONE,
    TH_FAKE_SSL_CALL_HANDSHAKE,
    TH_FAKE_SSL_CALL_READ,
    TH_FAKE_SSL_CALL_WRITE,
} th_fake_ssl_call;

typedef struct th_fake_ssl_ops {
    th_ssl_ops base;
    th_fake_ssl_call last_call;
    int handshake_ret;
    int handshake_error;
    int read_ret;
    int read_error;
    char read_data[64];
    int write_ret;
    int write_error;
    char written[64];
    size_t written_len;
    bool freed;
    BIO* rbio;
    BIO* wbio;
} th_fake_ssl_ops;

static SSL*
th_fake_new_ssl(void* self, SSL_CTX* ctx)
{
    (void)self;
    (void)ctx;
    return TH_FAKE_SSL;
}

static void
th_fake_free_ssl(void* self, SSL* ssl)
{
    (void)ssl;
    th_fake_ssl_ops* ops = self;
    /* Mirrors real SSL_free: SSL_set_bio transferred ownership of
     * rbio/wbio to the SSL object, so freeing them is SSL_free's job. */
    BIO_free(ops->rbio);
    BIO_free(ops->wbio);
    ops->freed = true;
}

static void
th_fake_set_bio(void* self, SSL* ssl, BIO* rbio, BIO* wbio)
{
    (void)ssl;
    th_fake_ssl_ops* ops = self;
    ops->rbio = rbio;
    ops->wbio = wbio;
}

static void
th_fake_set_accept_state(void* self, SSL* ssl)
{
    (void)self;
    (void)ssl;
}

static void
th_fake_set_partial_write(void* self, SSL* ssl)
{
    (void)self;
    (void)ssl;
}

static int
th_fake_do_handshake(void* self, SSL* ssl)
{
    (void)ssl;
    th_fake_ssl_ops* ops = self;
    ops->last_call = TH_FAKE_SSL_CALL_HANDSHAKE;
    return ops->handshake_ret;
}

static int
th_fake_read(void* self, SSL* ssl, void* buf, int len)
{
    (void)ssl;
    th_fake_ssl_ops* ops = self;
    ops->last_call = TH_FAKE_SSL_CALL_READ;
    if (ops->read_ret > 0) {
        size_t n = (size_t)ops->read_ret;
        memcpy(buf, ops->read_data, n < (size_t)len ? n : (size_t)len);
    }
    return ops->read_ret;
}

static int
th_fake_write(void* self, SSL* ssl, const void* buf, int len)
{
    (void)ssl;
    th_fake_ssl_ops* ops = self;
    ops->last_call = TH_FAKE_SSL_CALL_WRITE;
    if (ops->write_ret > 0) {
        size_t n = (size_t)ops->write_ret;
        memcpy(ops->written + ops->written_len, buf, n < (size_t)len ? n : (size_t)len);
        ops->written_len += n;
    }
    return ops->write_ret;
}

static int
th_fake_get_error(void* self, SSL* ssl, int ret)
{
    (void)ssl;
    (void)ret;
    th_fake_ssl_ops* ops = self;
    switch (ops->last_call) {
    case TH_FAKE_SSL_CALL_HANDSHAKE:
        return ops->handshake_error;
    case TH_FAKE_SSL_CALL_READ:
        return ops->read_error;
    case TH_FAKE_SSL_CALL_WRITE:
        return ops->write_error;
    default:
        return SSL_ERROR_NONE;
    }
}

static void
th_fake_ssl_ops_init(th_fake_ssl_ops* ops)
{
    ops->base.new_ssl = th_fake_new_ssl;
    ops->base.free_ssl = th_fake_free_ssl;
    ops->base.set_bio = th_fake_set_bio;
    ops->base.set_accept_state = th_fake_set_accept_state;
    ops->base.set_partial_write = th_fake_set_partial_write;
    ops->base.do_handshake = th_fake_do_handshake;
    ops->base.read = th_fake_read;
    ops->base.write = th_fake_write;
    ops->base.get_error = th_fake_get_error;
    ops->last_call = TH_FAKE_SSL_CALL_NONE;
    ops->handshake_ret = 1;
    ops->handshake_error = SSL_ERROR_NONE;
    ops->read_ret = 0;
    ops->read_error = SSL_ERROR_NONE;
    ops->write_ret = 0;
    ops->write_error = SSL_ERROR_NONE;
    ops->written_len = 0;
    ops->freed = false;
    ops->rbio = NULL;
    ops->wbio = NULL;
}

TH_TEST_BEGIN(ssl_session)
{
    th_fake_ssl_ops ops;
    th_fake_ssl_ops_init(&ops);
    th_ssl_context context;
    context.ctx = NULL;
    context.smem_method = NULL;
    context.ops = &ops.base;

    TH_TEST_CASE_BEGIN(ssl_session_handshake_done_when_ssl_reports_success)
    {
        th_ssl_session session;
        TH_EXPECT(th_ssl_session_init(&session, &context, &ops.base, NULL) == TH_ERR_OK);

        th_err err = TH_ERR_OK;
        th_ssl_result result = th_ssl_session_handshake(&session, &err);
        TH_EXPECT(result == TH_SSL_DONE);
        TH_EXPECT(err == TH_ERR_OK);

        th_ssl_session_deinit(&session);
        TH_EXPECT(ops.freed);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(ssl_session_handshake_wants_read_on_want_read)
    {
        ops.handshake_ret = -1;
        ops.handshake_error = SSL_ERROR_WANT_READ;
        th_ssl_session session;
        TH_EXPECT(th_ssl_session_init(&session, &context, &ops.base, NULL) == TH_ERR_OK);

        th_err err = TH_ERR_OK;
        th_ssl_result result = th_ssl_session_handshake(&session, &err);
        TH_EXPECT(result == TH_SSL_WANT_READ);
        TH_EXPECT(err == TH_ERR_OK);

        th_ssl_session_deinit(&session);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(ssl_session_handshake_wants_write_when_wbio_has_pending_data)
    {
        ops.handshake_ret = -1;
        ops.handshake_error = SSL_ERROR_WANT_READ;
        th_ssl_session session;
        TH_EXPECT(th_ssl_session_init(&session, &context, &ops.base, NULL) == TH_ERR_OK);
        BIO_write(session.wbio, "hi", 2);

        th_err err = TH_ERR_OK;
        th_ssl_result result = th_ssl_session_handshake(&session, &err);
        TH_EXPECT(result == TH_SSL_WANT_WRITE);
        TH_EXPECT(err == TH_ERR_OK);

        th_ssl_session_deinit(&session);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(ssl_session_handshake_reports_error_on_ssl_failure)
    {
        ops.handshake_ret = -1;
        ops.handshake_error = SSL_ERROR_SSL;
        th_ssl_session session;
        TH_EXPECT(th_ssl_session_init(&session, &context, &ops.base, NULL) == TH_ERR_OK);

        th_err err = TH_ERR_OK;
        th_ssl_result result = th_ssl_session_handshake(&session, &err);
        TH_EXPECT(result == TH_SSL_ERROR);
        TH_EXPECT(err == TH_ERR_SSL(SSL_ERROR_SSL));

        th_ssl_session_deinit(&session);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(ssl_session_read_returns_plaintext_on_success)
    {
        ops.read_ret = 5;
        memcpy(ops.read_data, "hello", 5);
        th_ssl_session session;
        TH_EXPECT(th_ssl_session_init(&session, &context, &ops.base, NULL) == TH_ERR_OK);

        char buf[16] = {0};
        size_t out = 0;
        th_err err = TH_ERR_OK;
        th_ssl_result result = th_ssl_session_read(&session, buf, sizeof(buf), &out, &err);
        TH_EXPECT(result == TH_SSL_DONE);
        TH_EXPECT(err == TH_ERR_OK);
        TH_EXPECT(out == 5);
        TH_EXPECT(memcmp(buf, "hello", 5) == 0);

        th_ssl_session_deinit(&session);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(ssl_session_read_zero_return_is_eof)
    {
        ops.read_ret = -1;
        ops.read_error = SSL_ERROR_ZERO_RETURN;
        th_ssl_session session;
        TH_EXPECT(th_ssl_session_init(&session, &context, &ops.base, NULL) == TH_ERR_OK);

        char buf[16] = {0};
        size_t out = 0;
        th_err err = TH_ERR_OK;
        th_ssl_result result = th_ssl_session_read(&session, buf, sizeof(buf), &out, &err);
        TH_EXPECT(result == TH_SSL_ERROR);
        TH_EXPECT(err == TH_ERR_EOF);

        th_ssl_session_deinit(&session);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(ssl_session_write_sends_ciphertext_and_wants_write)
    {
        ops.write_ret = 5;
        th_ssl_session session;
        TH_EXPECT(th_ssl_session_init(&session, &context, &ops.base, NULL) == TH_ERR_OK);

        size_t out = 0;
        th_err err = TH_ERR_OK;
        th_ssl_result result = th_ssl_session_write(&session, "hello", 5, &out, &err);
        TH_EXPECT(result == TH_SSL_WANT_WRITE);
        TH_EXPECT(err == TH_ERR_OK);
        TH_EXPECT(out == 5);
        TH_EXPECT(memcmp(ops.written, "hello", 5) == 0);

        th_ssl_session_deinit(&session);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(ssl_session_ciphertext_out_reflects_wbio_contents)
    {
        th_ssl_session session;
        TH_EXPECT(th_ssl_session_init(&session, &context, &ops.base, NULL) == TH_ERR_OK);
        BIO_write(session.wbio, "cipher", 6);

        th_iov iov;
        th_ssl_session_get_ciphertext_out(&session, &iov);
        TH_EXPECT(iov.len == 6);
        TH_EXPECT(memcmp(iov.base, "cipher", 6) == 0);

        th_ssl_session_consume_ciphertext_out(&session, 6);
        th_ssl_session_get_ciphertext_out(&session, &iov);
        TH_EXPECT(iov.len == 0);

        th_ssl_session_deinit(&session);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(ssl_session_ciphertext_in_is_visible_to_reads_after_feeding)
    {
        ops.read_ret = -1;
        ops.read_error = SSL_ERROR_WANT_READ;
        th_ssl_session session;
        TH_EXPECT(th_ssl_session_init(&session, &context, &ops.base, NULL) == TH_ERR_OK);

        th_iov iov;
        th_ssl_session_get_ciphertext_in_buf(&session, &iov);
        TH_EXPECT(iov.len > 0);
        memcpy(iov.base, "cipher", 6);
        th_ssl_session_fed_ciphertext_in(&session, 6);

        TH_EXPECT(BIO_pending(session.rbio) == 6);

        th_ssl_session_deinit(&session);
    }
    TH_TEST_CASE_END

    th_ssl_context_deinit(&context);
}
TH_TEST_END

#endif
