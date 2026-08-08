#include "th_config.h"

#if TH_WITH_SSL

#include "th_ssl_io.h"
#include "th_system_error.h"
#include "th_test.h"

#include <string.h>

static int th_fake_ssl_instance;
#define TH_FAKE_SSL ((SSL*)&th_fake_ssl_instance)

typedef struct th_fake_handle {
    th_handle base;
    int fd;
} th_fake_handle;

static th_err
th_fake_handle_submit(void* self, th_op* op)
{
    (void)self;
    th_op_perform(op);
    return TH_ERR_OK;
}

static void
th_fake_handle_enable_timeout(void* self, bool enabled)
{
    (void)self;
    (void)enabled;
}

static int
th_fake_handle_get_fd(const void* self)
{
    const th_fake_handle* handle = self;
    return handle->fd;
}

static void
th_fake_handle_destroy(void* self)
{
    (void)self;
}

static const th_handle_methods th_fake_handle_methods = {
    .cancel = NULL,
    .submit = th_fake_handle_submit,
    .enable_timeout = th_fake_handle_enable_timeout,
    .get_fd = th_fake_handle_get_fd,
    .destroy = th_fake_handle_destroy,
};

typedef struct th_fake_reactor {
    th_reactor base;
    th_fake_handle handle;
} th_fake_reactor;

static th_err
th_fake_reactor_create_handle(void* self, th_handle** out, int fd)
{
    th_fake_reactor* reactor = self;
    reactor->handle.base.methods = &th_fake_handle_methods;
    reactor->handle.fd = fd;
    *out = &reactor->handle.base;
    return TH_ERR_OK;
}

static void
th_fake_reactor_run(void* self, int timeout_ms)
{
    (void)self;
    (void)timeout_ms;
}

static const th_reactor_methods th_fake_reactor_methods = {
    .run = th_fake_reactor_run,
    .create_handle = th_fake_reactor_create_handle,
    .destroy = NULL,
};

static void
th_fake_reactor_init(th_fake_reactor* reactor)
{
    reactor->base.methods = &th_fake_reactor_methods;
}

/* Raw ciphertext transport: a plain byte pipe standing in for the peer. */
typedef struct th_fake_socket_ops {
    th_socket_ops base;
    char written[256];
    size_t written_len;
    char to_read[256];
    size_t to_read_len;
    size_t read_pos;
} th_fake_socket_ops;

static th_err
th_fake_sendvec(void* self, int fd, const th_iov* iov, size_t iovcnt, size_t* result)
{
    (void)fd;
    th_fake_socket_ops* ops = self;
    size_t total = 0;
    for (size_t i = 0; i < iovcnt; ++i) {
        memcpy(ops->written + ops->written_len, iov[i].base, iov[i].len);
        ops->written_len += iov[i].len;
        total += iov[i].len;
    }
    *result = total;
    return TH_ERR_OK;
}

static th_err
th_fake_recv(void* self, int fd, void* addr, size_t len, size_t* result)
{
    (void)fd;
    th_fake_socket_ops* ops = self;
    size_t avail = ops->to_read_len - ops->read_pos;
    size_t n = len < avail ? len : avail;
    memcpy(addr, ops->to_read + ops->read_pos, n);
    ops->read_pos += n;
    *result = n;
    return TH_ERR_OK;
}

static void
th_fake_socket_ops_init(th_fake_socket_ops* ops)
{
    ops->base.send = NULL;
    ops->base.sendvec = th_fake_sendvec;
    ops->base.recv = th_fake_recv;
    ops->base.sendfile = NULL;
    ops->written_len = 0;
    ops->to_read_len = 0;
    ops->read_pos = 0;
}

/* SSL layer: entirely faked, drives th_ssl_session's steps directly. */
typedef enum th_fake_ssl_call {
    TH_FAKE_SSL_CALL_NONE,
    TH_FAKE_SSL_CALL_HANDSHAKE,
    TH_FAKE_SSL_CALL_READ,
    TH_FAKE_SSL_CALL_WRITE,
} th_fake_ssl_call;

typedef struct th_fake_ssl_ops {
    th_ssl_ops base;
    th_fake_ssl_call last_call;
    /* Each queue is consumed front-to-back, one entry per SSL_* call, so
     * a test can script e.g. WANT_READ then success across retries. */
    int handshake_rets[4];
    int handshake_errors[4];
    size_t handshake_idx;
    int read_rets[4];
    int read_errors[4];
    char read_data[64];
    size_t read_idx;
    int write_rets[4];
    int write_errors[4];
    size_t write_idx;
    char written[64];
    size_t written_len;
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
    return ops->handshake_rets[ops->handshake_idx++];
}

static int
th_fake_read(void* self, SSL* ssl, void* buf, int len)
{
    (void)ssl;
    th_fake_ssl_ops* ops = self;
    ops->last_call = TH_FAKE_SSL_CALL_READ;
    int ret = ops->read_rets[ops->read_idx++];
    if (ret > 0) {
        size_t n = (size_t)ret;
        memcpy(buf, ops->read_data, n < (size_t)len ? n : (size_t)len);
    }
    return ret;
}

static int
th_fake_write(void* self, SSL* ssl, const void* buf, int len)
{
    (void)ssl;
    th_fake_ssl_ops* ops = self;
    ops->last_call = TH_FAKE_SSL_CALL_WRITE;
    int ret = ops->write_rets[ops->write_idx++];
    if (ret > 0) {
        size_t n = (size_t)ret;
        memcpy(ops->written + ops->written_len, buf, n < (size_t)len ? n : (size_t)len);
        ops->written_len += n;
    }
    return ret;
}

static int
th_fake_get_error(void* self, SSL* ssl, int ret)
{
    (void)ssl;
    (void)ret;
    th_fake_ssl_ops* ops = self;
    switch (ops->last_call) {
    case TH_FAKE_SSL_CALL_HANDSHAKE:
        return ops->handshake_errors[ops->handshake_idx - 1];
    case TH_FAKE_SSL_CALL_READ:
        return ops->read_errors[ops->read_idx - 1];
    case TH_FAKE_SSL_CALL_WRITE:
        return ops->write_errors[ops->write_idx - 1];
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
    memset(ops->handshake_rets, 0, sizeof(ops->handshake_rets));
    memset(ops->handshake_errors, 0, sizeof(ops->handshake_errors));
    ops->handshake_idx = 0;
    memset(ops->read_rets, 0, sizeof(ops->read_rets));
    memset(ops->read_errors, 0, sizeof(ops->read_errors));
    ops->read_idx = 0;
    memset(ops->write_rets, 0, sizeof(ops->write_rets));
    memset(ops->write_errors, 0, sizeof(ops->write_errors));
    ops->write_idx = 0;
    ops->written_len = 0;
    ops->rbio = NULL;
    ops->wbio = NULL;
}

typedef struct th_recorded_io {
    bool called;
    size_t result;
    th_err err;
} th_recorded_io;

static void
th_recorded_io_cb(void* user_data, size_t size, th_err err)
{
    th_recorded_io* result = user_data;
    result->called = true;
    result->result = size;
    result->err = err;
}

static void
th_recorded_io_init(th_recorded_io* result)
{
    result->called = false;
    result->result = 0;
    result->err = TH_ERR_OK;
}

TH_TEST_BEGIN(ssl_io)
{
    /* context.ctx stays NULL in every case below (no real SSL_CTX is
     * ever created), so context.ops is never dereferenced. */
    th_ssl_context context;
    context.ctx = NULL;
    context.smem_method = NULL;
    context.ops = NULL;

    TH_TEST_CASE_BEGIN(ssl_io_handshake_completes_immediately_when_ssl_says_done)
    {
        th_fake_reactor reactor;
        th_fake_reactor_init(&reactor);
        th_loop loop;
        th_loop_init(&loop, &reactor.base);
        th_fake_socket_ops socket_ops;
        th_fake_socket_ops_init(&socket_ops);
        th_socket socket;
        th_socket_init(&socket, &loop, &socket_ops.base);
        TH_EXPECT(th_socket_set_fd(&socket, 5) == TH_ERR_OK);

        th_fake_ssl_ops ssl_ops;
        th_fake_ssl_ops_init(&ssl_ops);
        ssl_ops.handshake_rets[0] = 1;
        th_ssl_session session;
        TH_EXPECT(th_ssl_session_init(&session, &context, &ssl_ops.base, NULL) == TH_ERR_OK);

        th_recorded_io result;
        th_recorded_io_init(&result);
        th_ssl_io_op op;
        th_ssl_io_op_init_handshake(&op, &socket, &session, th_recorded_io_cb, &result);
        th_op_perform(&op.base);
        th_loop_run(&loop);

        TH_EXPECT(result.called);
        TH_EXPECT(result.err == TH_ERR_OK);

        th_ssl_session_deinit(&session);
        th_loop_deinit(&loop);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(ssl_io_handshake_reads_more_ciphertext_on_want_read_then_succeeds)
    {
        th_fake_reactor reactor;
        th_fake_reactor_init(&reactor);
        th_loop loop;
        th_loop_init(&loop, &reactor.base);
        th_fake_socket_ops socket_ops;
        th_fake_socket_ops_init(&socket_ops);
        memcpy(socket_ops.to_read, "clienthello", 11);
        socket_ops.to_read_len = 11;
        th_socket socket;
        th_socket_init(&socket, &loop, &socket_ops.base);
        TH_EXPECT(th_socket_set_fd(&socket, 5) == TH_ERR_OK);

        th_fake_ssl_ops ssl_ops;
        th_fake_ssl_ops_init(&ssl_ops);
        ssl_ops.handshake_rets[0] = -1;
        ssl_ops.handshake_errors[0] = SSL_ERROR_WANT_READ;
        ssl_ops.handshake_rets[1] = 1;
        th_ssl_session session;
        TH_EXPECT(th_ssl_session_init(&session, &context, &ssl_ops.base, NULL) == TH_ERR_OK);

        th_recorded_io result;
        th_recorded_io_init(&result);
        th_ssl_io_op op;
        th_ssl_io_op_init_handshake(&op, &socket, &session, th_recorded_io_cb, &result);
        th_op_perform(&op.base);
        th_loop_run(&loop);

        TH_EXPECT(result.called);
        TH_EXPECT(result.err == TH_ERR_OK);
        TH_EXPECT(ssl_ops.handshake_idx == 2);
        TH_EXPECT(BIO_pending(session.rbio) == 11);

        th_ssl_session_deinit(&session);
        th_loop_deinit(&loop);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(ssl_io_handshake_drains_ciphertext_out_on_want_write_then_succeeds)
    {
        th_fake_reactor reactor;
        th_fake_reactor_init(&reactor);
        th_loop loop;
        th_loop_init(&loop, &reactor.base);
        th_fake_socket_ops socket_ops;
        th_fake_socket_ops_init(&socket_ops);
        th_socket socket;
        th_socket_init(&socket, &loop, &socket_ops.base);
        TH_EXPECT(th_socket_set_fd(&socket, 5) == TH_ERR_OK);

        th_fake_ssl_ops ssl_ops;
        th_fake_ssl_ops_init(&ssl_ops);
        ssl_ops.handshake_rets[0] = -1;
        ssl_ops.handshake_errors[0] = SSL_ERROR_WANT_READ; /* irrelevant: wbio pending wins */
        ssl_ops.handshake_rets[1] = 1;
        th_ssl_session session;
        TH_EXPECT(th_ssl_session_init(&session, &context, &ssl_ops.base, NULL) == TH_ERR_OK);
        BIO_write(session.wbio, "serverhello", 11);

        th_recorded_io result;
        th_recorded_io_init(&result);
        th_ssl_io_op op;
        th_ssl_io_op_init_handshake(&op, &socket, &session, th_recorded_io_cb, &result);
        th_op_perform(&op.base);
        th_loop_run(&loop);

        TH_EXPECT(result.called);
        TH_EXPECT(result.err == TH_ERR_OK);
        TH_EXPECT(memcmp(socket_ops.written, "serverhello", 11) == 0);

        th_ssl_session_deinit(&session);
        th_loop_deinit(&loop);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(ssl_io_handshake_reports_error_on_ssl_failure)
    {
        th_fake_reactor reactor;
        th_fake_reactor_init(&reactor);
        th_loop loop;
        th_loop_init(&loop, &reactor.base);
        th_fake_socket_ops socket_ops;
        th_fake_socket_ops_init(&socket_ops);
        th_socket socket;
        th_socket_init(&socket, &loop, &socket_ops.base);
        TH_EXPECT(th_socket_set_fd(&socket, 5) == TH_ERR_OK);

        th_fake_ssl_ops ssl_ops;
        th_fake_ssl_ops_init(&ssl_ops);
        ssl_ops.handshake_rets[0] = -1;
        ssl_ops.handshake_errors[0] = SSL_ERROR_SSL;
        th_ssl_session session;
        TH_EXPECT(th_ssl_session_init(&session, &context, &ssl_ops.base, NULL) == TH_ERR_OK);

        th_recorded_io result;
        th_recorded_io_init(&result);
        th_ssl_io_op op;
        th_ssl_io_op_init_handshake(&op, &socket, &session, th_recorded_io_cb, &result);
        th_op_perform(&op.base);
        th_loop_run(&loop);

        TH_EXPECT(result.called);
        TH_EXPECT(result.err == TH_ERR_SSL(SSL_ERROR_SSL));

        th_ssl_session_deinit(&session);
        th_loop_deinit(&loop);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(ssl_io_read_returns_plaintext_produced_by_one_ssl_read)
    {
        th_fake_reactor reactor;
        th_fake_reactor_init(&reactor);
        th_loop loop;
        th_loop_init(&loop, &reactor.base);
        th_fake_socket_ops socket_ops;
        th_fake_socket_ops_init(&socket_ops);
        th_socket socket;
        th_socket_init(&socket, &loop, &socket_ops.base);
        TH_EXPECT(th_socket_set_fd(&socket, 5) == TH_ERR_OK);

        th_fake_ssl_ops ssl_ops;
        th_fake_ssl_ops_init(&ssl_ops);
        ssl_ops.read_rets[0] = 5;
        memcpy(ssl_ops.read_data, "hello", 5);
        th_ssl_session session;
        TH_EXPECT(th_ssl_session_init(&session, &context, &ssl_ops.base, NULL) == TH_ERR_OK);

        char buf[16] = {0};
        th_recorded_io result;
        th_recorded_io_init(&result);
        th_ssl_io_op op;
        th_ssl_io_op_init_read(&op, &socket, &session, buf, sizeof(buf), th_recorded_io_cb, &result);
        th_op_perform(&op.base);
        th_loop_run(&loop);

        TH_EXPECT(result.called);
        TH_EXPECT(result.err == TH_ERR_OK);
        TH_EXPECT(result.result == 5);
        TH_EXPECT(memcmp(buf, "hello", 5) == 0);
        TH_EXPECT(ssl_ops.read_idx == 1); /* did not call SSL_read again after making progress */

        th_ssl_session_deinit(&session);
        th_loop_deinit(&loop);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(ssl_io_read_waits_for_more_ciphertext_then_returns_plaintext)
    {
        th_fake_reactor reactor;
        th_fake_reactor_init(&reactor);
        th_loop loop;
        th_loop_init(&loop, &reactor.base);
        th_fake_socket_ops socket_ops;
        th_fake_socket_ops_init(&socket_ops);
        memcpy(socket_ops.to_read, "ciphertext", 10);
        socket_ops.to_read_len = 10;
        th_socket socket;
        th_socket_init(&socket, &loop, &socket_ops.base);
        TH_EXPECT(th_socket_set_fd(&socket, 5) == TH_ERR_OK);

        th_fake_ssl_ops ssl_ops;
        th_fake_ssl_ops_init(&ssl_ops);
        ssl_ops.read_rets[0] = -1;
        ssl_ops.read_errors[0] = SSL_ERROR_WANT_READ;
        ssl_ops.read_rets[1] = 5;
        memcpy(ssl_ops.read_data, "hello", 5);
        th_ssl_session session;
        TH_EXPECT(th_ssl_session_init(&session, &context, &ssl_ops.base, NULL) == TH_ERR_OK);

        char buf[16] = {0};
        th_recorded_io result;
        th_recorded_io_init(&result);
        th_ssl_io_op op;
        th_ssl_io_op_init_read(&op, &socket, &session, buf, sizeof(buf), th_recorded_io_cb, &result);
        th_op_perform(&op.base);
        th_loop_run(&loop);

        TH_EXPECT(result.called);
        TH_EXPECT(result.err == TH_ERR_OK);
        TH_EXPECT(result.result == 5);
        TH_EXPECT(memcmp(buf, "hello", 5) == 0);

        th_ssl_session_deinit(&session);
        th_loop_deinit(&loop);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(ssl_io_read_zero_return_completes_with_eof)
    {
        th_fake_reactor reactor;
        th_fake_reactor_init(&reactor);
        th_loop loop;
        th_loop_init(&loop, &reactor.base);
        th_fake_socket_ops socket_ops;
        th_fake_socket_ops_init(&socket_ops);
        th_socket socket;
        th_socket_init(&socket, &loop, &socket_ops.base);
        TH_EXPECT(th_socket_set_fd(&socket, 5) == TH_ERR_OK);

        th_fake_ssl_ops ssl_ops;
        th_fake_ssl_ops_init(&ssl_ops);
        ssl_ops.read_rets[0] = -1;
        ssl_ops.read_errors[0] = SSL_ERROR_ZERO_RETURN;
        th_ssl_session session;
        TH_EXPECT(th_ssl_session_init(&session, &context, &ssl_ops.base, NULL) == TH_ERR_OK);

        char buf[16] = {0};
        th_recorded_io result;
        th_recorded_io_init(&result);
        th_ssl_io_op op;
        th_ssl_io_op_init_read(&op, &socket, &session, buf, sizeof(buf), th_recorded_io_cb, &result);
        th_op_perform(&op.base);
        th_loop_run(&loop);

        TH_EXPECT(result.called);
        TH_EXPECT(result.err == TH_ERR_EOF);

        th_ssl_session_deinit(&session);
        th_loop_deinit(&loop);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(ssl_io_write_sends_ciphertext_and_completes)
    {
        th_fake_reactor reactor;
        th_fake_reactor_init(&reactor);
        th_loop loop;
        th_loop_init(&loop, &reactor.base);
        th_fake_socket_ops socket_ops;
        th_fake_socket_ops_init(&socket_ops);
        th_socket socket;
        th_socket_init(&socket, &loop, &socket_ops.base);
        TH_EXPECT(th_socket_set_fd(&socket, 5) == TH_ERR_OK);

        th_fake_ssl_ops ssl_ops;
        th_fake_ssl_ops_init(&ssl_ops);
        ssl_ops.write_rets[0] = 5;
        th_ssl_session session;
        TH_EXPECT(th_ssl_session_init(&session, &context, &ssl_ops.base, NULL) == TH_ERR_OK);
        BIO_write(session.wbio, "cipher", 6);

        th_recorded_io result;
        th_recorded_io_init(&result);
        th_ssl_io_op op;
        th_ssl_io_op_init_write(&op, &socket, &session, "hello", 5, th_recorded_io_cb, &result);
        th_op_perform(&op.base);
        th_loop_run(&loop);

        TH_EXPECT(result.called);
        TH_EXPECT(result.err == TH_ERR_OK);
        TH_EXPECT(result.result == 5);
        TH_EXPECT(memcmp(ssl_ops.written, "hello", 5) == 0);
        TH_EXPECT(memcmp(socket_ops.written, "cipher", 6) == 0);

        th_ssl_session_deinit(&session);
        th_loop_deinit(&loop);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(ssl_io_abort_completes_with_given_error)
    {
        th_fake_reactor reactor;
        th_fake_reactor_init(&reactor);
        th_loop loop;
        th_loop_init(&loop, &reactor.base);
        th_fake_socket_ops socket_ops;
        th_fake_socket_ops_init(&socket_ops);
        th_socket socket;
        th_socket_init(&socket, &loop, &socket_ops.base);
        TH_EXPECT(th_socket_set_fd(&socket, 5) == TH_ERR_OK);

        th_fake_ssl_ops ssl_ops;
        th_fake_ssl_ops_init(&ssl_ops);
        th_ssl_session session;
        TH_EXPECT(th_ssl_session_init(&session, &context, &ssl_ops.base, NULL) == TH_ERR_OK);

        char buf[16] = {0};
        th_recorded_io result;
        th_recorded_io_init(&result);
        th_ssl_io_op op;
        th_ssl_io_op_init_read(&op, &socket, &session, buf, sizeof(buf), th_recorded_io_cb, &result);
        th_op_abort(&op.base, TH_ERR_SYSTEM(TH_ECANCELED));
        th_loop_run(&loop);

        TH_EXPECT(result.called);
        TH_EXPECT(result.err == TH_ERR_SYSTEM(TH_ECANCELED));

        th_ssl_session_deinit(&session);
        th_loop_deinit(&loop);
    }
    TH_TEST_CASE_END

    th_ssl_context_deinit(&context);
}
TH_TEST_END

#endif
