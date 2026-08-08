#include "th_config.h"

#if TH_WITH_SSL

#include "th_ssl_conn.h"
#include "th_system_error.h"
#include "th_test.h"

#include <string.h>

static int th_fake_ssl_instance;
#define TH_FAKE_SSL ((SSL*)&th_fake_ssl_instance)

typedef struct th_fake_handle {
    th_handle base;
    int fd;
    bool cancelled;
} th_fake_handle;

static void
th_fake_handle_cancel(void* self)
{
    th_fake_handle* handle = self;
    handle->cancelled = true;
}

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
    .cancel = th_fake_handle_cancel,
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
    reactor->handle.cancelled = false;
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

typedef struct th_fake_socket_ops {
    th_socket_ops base;
    char written[256];
    size_t written_len;
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
    (void)self;
    (void)fd;
    (void)addr;
    (void)len;
    *result = 0;
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
}

typedef struct th_fake_ssl_ops {
    th_ssl_ops base;
    int handshake_ret;
    int handshake_error;
    int read_ret;
    int read_error;
    char read_data[64];
    int write_ret;
    char written[64];
    size_t written_len;
    bool last_was_read;
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
    return ops->handshake_ret;
}

static int
th_fake_read(void* self, SSL* ssl, void* buf, int len)
{
    (void)ssl;
    th_fake_ssl_ops* ops = self;
    ops->last_was_read = true;
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
    ops->last_was_read = false;
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
    return ops->last_was_read ? ops->read_error : ops->handshake_error;
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
    ops->handshake_ret = 1;
    ops->handshake_error = SSL_ERROR_NONE;
    ops->read_ret = 0;
    ops->read_error = SSL_ERROR_NONE;
    ops->write_ret = 0;
    ops->written_len = 0;
    ops->last_was_read = false;
    ops->rbio = NULL;
    ops->wbio = NULL;
}

typedef struct th_recording_observer {
    th_conn_observer base;
    int init_count;
    int deinit_count;
} th_recording_observer;

static void
th_recording_observer_on_init(th_conn_observer* self, th_conn_observable* observable)
{
    (void)observable;
    th_recording_observer* observer = (th_recording_observer*)self;
    ++observer->init_count;
}

static void
th_recording_observer_on_deinit(th_conn_observer* self, th_conn_observable* observable)
{
    (void)observable;
    th_recording_observer* observer = (th_recording_observer*)self;
    ++observer->deinit_count;
}

static void
th_recording_observer_init(th_recording_observer* observer)
{
    observer->base.on_init = th_recording_observer_on_init;
    observer->base.on_deinit = th_recording_observer_on_deinit;
    observer->init_count = 0;
    observer->deinit_count = 0;
}

static int th_upgrade_calls = 0;

static void
th_test_upgrade_fn(void* self, th_conn* conn)
{
    (void)self;
    (void)conn;
    ++th_upgrade_calls;
}

typedef struct th_recorded_send {
    bool called;
    size_t result;
    th_err err;
} th_recorded_send;

static void
th_recorded_send_cb(void* user_data, size_t size, th_err err)
{
    th_recorded_send* result = user_data;
    result->called = true;
    result->result = size;
    result->err = err;
}

static void
th_recorded_send_init(th_recorded_send* result)
{
    result->called = false;
    result->result = 0;
    result->err = TH_ERR_OK;
}

typedef struct th_recorded_recv {
    bool called;
    size_t result;
    th_err err;
} th_recorded_recv;

static void
th_recorded_recv_cb(void* user_data, size_t size, th_err err)
{
    th_recorded_recv* result = user_data;
    result->called = true;
    result->result = size;
    result->err = err;
}

static void
th_recorded_recv_init(th_recorded_recv* result)
{
    result->called = false;
    result->result = 0;
    result->err = TH_ERR_OK;
}

TH_TEST_BEGIN(ssl_conn)
{
    /* context.ctx stays NULL in every case below (no real SSL_CTX is
     * ever created), so context.ops is never dereferenced. */
    th_ssl_context context;
    context.ctx = NULL;
    context.smem_method = NULL;
    context.ops = NULL;

    TH_TEST_CASE_BEGIN(ssl_conn_start_runs_handshake_then_calls_upgrader)
    {
        th_upgrade_calls = 0;
        th_fake_reactor reactor;
        th_fake_reactor_init(&reactor);
        th_loop loop;
        th_loop_init(&loop, &reactor.base);
        th_fake_socket_ops socket_ops;
        th_fake_socket_ops_init(&socket_ops);
        th_socket socket;
        th_socket_init(&socket, &loop, &socket_ops.base);

        th_fake_ssl_ops ssl_ops;
        th_fake_ssl_ops_init(&ssl_ops);

        th_recording_observer observer;
        th_recording_observer_init(&observer);
        th_conn_upgrader upgrader;
        th_conn_upgrader_init(&upgrader, th_test_upgrade_fn);

        th_conn* conn = NULL;
        TH_EXPECT(th_ssl_conn_create(&conn, &socket, &context, &ssl_ops.base, &upgrader, &observer.base, NULL) == TH_ERR_OK);
        TH_EXPECT(observer.init_count == 1);
        TH_EXPECT(th_socket_set_fd(th_conn_get_socket(conn), 5) == TH_ERR_OK);

        th_conn_start(conn);
        th_loop_run(&loop);
        TH_EXPECT(th_upgrade_calls == 1);

        th_conn_destroy(conn);
        TH_EXPECT(observer.deinit_count == 1);
        th_loop_deinit(&loop);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(ssl_conn_start_destroys_conn_on_handshake_failure)
    {
        th_upgrade_calls = 0;
        th_fake_reactor reactor;
        th_fake_reactor_init(&reactor);
        th_loop loop;
        th_loop_init(&loop, &reactor.base);
        th_fake_socket_ops socket_ops;
        th_fake_socket_ops_init(&socket_ops);
        th_socket socket;
        th_socket_init(&socket, &loop, &socket_ops.base);

        th_fake_ssl_ops ssl_ops;
        th_fake_ssl_ops_init(&ssl_ops);
        ssl_ops.handshake_ret = -1;
        ssl_ops.handshake_error = SSL_ERROR_SSL;

        th_recording_observer observer;
        th_recording_observer_init(&observer);
        th_conn_upgrader upgrader;
        th_conn_upgrader_init(&upgrader, th_test_upgrade_fn);

        th_conn* conn = NULL;
        TH_EXPECT(th_ssl_conn_create(&conn, &socket, &context, &ssl_ops.base, &upgrader, &observer.base, NULL) == TH_ERR_OK);
        TH_EXPECT(th_socket_set_fd(th_conn_get_socket(conn), 5) == TH_ERR_OK);

        th_conn_start(conn);
        th_loop_run(&loop);
        TH_EXPECT(th_upgrade_calls == 0);
        TH_EXPECT(observer.deinit_count == 1);

        th_loop_deinit(&loop);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(ssl_conn_send_writes_plaintext_as_ciphertext_via_socket)
    {
        th_fake_reactor reactor;
        th_fake_reactor_init(&reactor);
        th_loop loop;
        th_loop_init(&loop, &reactor.base);
        th_fake_socket_ops socket_ops;
        th_fake_socket_ops_init(&socket_ops);
        th_socket socket;
        th_socket_init(&socket, &loop, &socket_ops.base);

        th_fake_ssl_ops ssl_ops;
        th_fake_ssl_ops_init(&ssl_ops);
        ssl_ops.write_ret = 5;

        th_recording_observer observer;
        th_recording_observer_init(&observer);
        th_conn_upgrader upgrader;
        th_conn_upgrader_init(&upgrader, th_test_upgrade_fn);

        th_conn* conn = NULL;
        TH_EXPECT(th_ssl_conn_create(&conn, &socket, &context, &ssl_ops.base, &upgrader, &observer.base, NULL) == TH_ERR_OK);
        TH_EXPECT(th_socket_set_fd(th_conn_get_socket(conn), 5) == TH_ERR_OK);

        th_iov iov[1] = {{(void*)"hello", 5}};
        th_recorded_send result;
        th_recorded_send_init(&result);
        th_conn_send(conn, iov, 1, NULL, 0, 0, th_recorded_send_cb, &result);
        th_loop_run(&loop);

        TH_EXPECT(result.called);
        TH_EXPECT(result.err == TH_ERR_OK);
        TH_EXPECT(result.result == 5);
        TH_EXPECT(memcmp(ssl_ops.written, "hello", 5) == 0);

        th_conn_destroy(conn);
        th_loop_deinit(&loop);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(ssl_conn_recv_reads_plaintext_via_socket)
    {
        th_fake_reactor reactor;
        th_fake_reactor_init(&reactor);
        th_loop loop;
        th_loop_init(&loop, &reactor.base);
        th_fake_socket_ops socket_ops;
        th_fake_socket_ops_init(&socket_ops);
        th_socket socket;
        th_socket_init(&socket, &loop, &socket_ops.base);

        th_fake_ssl_ops ssl_ops;
        th_fake_ssl_ops_init(&ssl_ops);
        ssl_ops.read_ret = 5;
        memcpy(ssl_ops.read_data, "world", 5);

        th_recording_observer observer;
        th_recording_observer_init(&observer);
        th_conn_upgrader upgrader;
        th_conn_upgrader_init(&upgrader, th_test_upgrade_fn);

        th_conn* conn = NULL;
        TH_EXPECT(th_ssl_conn_create(&conn, &socket, &context, &ssl_ops.base, &upgrader, &observer.base, NULL) == TH_ERR_OK);
        TH_EXPECT(th_socket_set_fd(th_conn_get_socket(conn), 5) == TH_ERR_OK);

        char buf[64] = {0};
        th_recorded_recv result;
        th_recorded_recv_init(&result);
        th_conn_recv(conn, buf, sizeof(buf), false, th_recorded_recv_cb, &result);
        th_loop_run(&loop);

        TH_EXPECT(result.called);
        TH_EXPECT(result.err == TH_ERR_OK);
        TH_EXPECT(result.result == 5);
        TH_EXPECT(memcmp(buf, "world", 5) == 0);

        th_conn_destroy(conn);
        th_loop_deinit(&loop);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(ssl_conn_cancel_forwards_to_socket)
    {
        th_fake_reactor reactor;
        th_fake_reactor_init(&reactor);
        th_loop loop;
        th_loop_init(&loop, &reactor.base);
        th_fake_socket_ops socket_ops;
        th_fake_socket_ops_init(&socket_ops);
        th_socket socket;
        th_socket_init(&socket, &loop, &socket_ops.base);

        th_fake_ssl_ops ssl_ops;
        th_fake_ssl_ops_init(&ssl_ops);

        th_recording_observer observer;
        th_recording_observer_init(&observer);
        th_conn_upgrader upgrader;
        th_conn_upgrader_init(&upgrader, th_test_upgrade_fn);

        th_conn* conn = NULL;
        TH_EXPECT(th_ssl_conn_create(&conn, &socket, &context, &ssl_ops.base, &upgrader, &observer.base, NULL) == TH_ERR_OK);
        TH_EXPECT(th_socket_set_fd(th_conn_get_socket(conn), 5) == TH_ERR_OK);

        th_conn_cancel(conn);
        TH_EXPECT(reactor.handle.cancelled);

        th_conn_destroy(conn);
        th_loop_deinit(&loop);
    }
    TH_TEST_CASE_END

    th_ssl_context_deinit(&context);
}
TH_TEST_END

#endif
