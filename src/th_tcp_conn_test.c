#include "th_system_error.h"
#include "th_tcp_conn.h"
#include "th_test.h"

#include <string.h>

typedef struct th_fake_handle {
    th_handle base;
    int fd;
    bool cancelled;
    bool destroyed;
} th_fake_handle;

static void
th_fake_handle_cancel(void* self)
{
    th_fake_handle* handle = self;
    handle->cancelled = true;
}

/* Simulates a reactor that is always immediately ready, driving each
 * op's retry loop synchronously instead of waiting for a real event. */
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
    th_fake_handle* handle = self;
    handle->destroyed = true;
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
    reactor->handle.destroyed = false;
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
    char written[64];
    size_t written_len;
    char to_read[64];
    size_t to_read_len;
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
    size_t n = len < ops->to_read_len ? len : ops->to_read_len;
    memcpy(addr, ops->to_read, n);
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

static int
    th_upgrade_calls = 0;

static void
th_test_upgrade_fn(void* self, th_conn* conn)
{
    (void)self;
    (void)conn;
    ++th_upgrade_calls;
}

TH_TEST_BEGIN(tcp_conn)
{
    TH_TEST_CASE_BEGIN(tcp_conn_start_calls_upgrader)
    {
        th_upgrade_calls = 0;
        th_fake_reactor reactor;
        th_fake_reactor_init(&reactor);
        th_loop loop;
        th_loop_init(&loop, &reactor.base);
        th_fake_socket_ops ops;
        th_fake_socket_ops_init(&ops);
        th_socket socket;
        th_socket_init(&socket, &loop, &ops.base);

        th_recording_observer observer;
        th_recording_observer_init(&observer);
        th_conn_upgrader upgrader;
        th_conn_upgrader_init(&upgrader, th_test_upgrade_fn);

        th_conn* conn = NULL;
        TH_EXPECT(th_tcp_conn_create(&conn, &socket, &upgrader, &observer.base, NULL) == TH_ERR_OK);
        TH_EXPECT(observer.init_count == 1);
        TH_EXPECT(th_socket_set_fd(th_conn_get_socket(conn), 5) == TH_ERR_OK);

        th_conn_start(conn);
        TH_EXPECT(th_upgrade_calls == 1);

        th_conn_destroy(conn);
        TH_EXPECT(observer.deinit_count == 1);
        th_loop_deinit(&loop);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(tcp_conn_get_address_is_zeroed_until_accept_fills_it)
    {
        th_fake_reactor reactor;
        th_fake_reactor_init(&reactor);
        th_loop loop;
        th_loop_init(&loop, &reactor.base);
        th_fake_socket_ops ops;
        th_fake_socket_ops_init(&ops);
        th_socket socket;
        th_socket_init(&socket, &loop, &ops.base);

        th_recording_observer observer;
        th_recording_observer_init(&observer);
        th_conn_upgrader upgrader;
        th_conn_upgrader_init(&upgrader, th_test_upgrade_fn);

        th_conn* conn = NULL;
        TH_EXPECT(th_tcp_conn_create(&conn, &socket, &upgrader, &observer.base, NULL) == TH_ERR_OK);
        TH_EXPECT(th_socket_set_fd(th_conn_get_socket(conn), 5) == TH_ERR_OK);

        th_address* addr = th_conn_get_address(conn);
        TH_EXPECT(addr != NULL);

        th_conn_destroy(conn);
        th_loop_deinit(&loop);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(tcp_conn_send_writes_iov_via_socket)
    {
        th_fake_reactor reactor;
        th_fake_reactor_init(&reactor);
        th_loop loop;
        th_loop_init(&loop, &reactor.base);
        th_fake_socket_ops ops;
        th_fake_socket_ops_init(&ops);
        th_socket socket;
        th_socket_init(&socket, &loop, &ops.base);

        th_recording_observer observer;
        th_recording_observer_init(&observer);
        th_conn_upgrader upgrader;
        th_conn_upgrader_init(&upgrader, th_test_upgrade_fn);

        th_conn* conn = NULL;
        TH_EXPECT(th_tcp_conn_create(&conn, &socket, &upgrader, &observer.base, NULL) == TH_ERR_OK);
        TH_EXPECT(th_socket_set_fd(th_conn_get_socket(conn), 5) == TH_ERR_OK);

        th_iov iov[1] = {{(void*)"hello", 5}};
        th_recorded_send result;
        th_recorded_send_init(&result);
        th_conn_send(conn, iov, 1, NULL, 0, 0, th_recorded_send_cb, &result);
        th_loop_run(&loop);

        TH_EXPECT(result.called);
        TH_EXPECT(result.err == TH_ERR_OK);
        TH_EXPECT(result.result == 5);
        TH_EXPECT(memcmp(ops.written, "hello", 5) == 0);

        th_conn_destroy(conn);
        th_loop_deinit(&loop);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(tcp_conn_recv_reads_via_socket)
    {
        th_fake_reactor reactor;
        th_fake_reactor_init(&reactor);
        th_loop loop;
        th_loop_init(&loop, &reactor.base);
        th_fake_socket_ops ops;
        th_fake_socket_ops_init(&ops);
        memcpy(ops.to_read, "world", 5);
        ops.to_read_len = 5;
        th_socket socket;
        th_socket_init(&socket, &loop, &ops.base);

        th_recording_observer observer;
        th_recording_observer_init(&observer);
        th_conn_upgrader upgrader;
        th_conn_upgrader_init(&upgrader, th_test_upgrade_fn);

        th_conn* conn = NULL;
        TH_EXPECT(th_tcp_conn_create(&conn, &socket, &upgrader, &observer.base, NULL) == TH_ERR_OK);
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
    TH_TEST_CASE_BEGIN(tcp_conn_cancel_forwards_to_socket)
    {
        th_fake_reactor reactor;
        th_fake_reactor_init(&reactor);
        th_loop loop;
        th_loop_init(&loop, &reactor.base);
        th_fake_socket_ops ops;
        th_fake_socket_ops_init(&ops);
        th_socket socket;
        th_socket_init(&socket, &loop, &ops.base);

        th_recording_observer observer;
        th_recording_observer_init(&observer);
        th_conn_upgrader upgrader;
        th_conn_upgrader_init(&upgrader, th_test_upgrade_fn);

        th_conn* conn = NULL;
        TH_EXPECT(th_tcp_conn_create(&conn, &socket, &upgrader, &observer.base, NULL) == TH_ERR_OK);
        TH_EXPECT(th_socket_set_fd(th_conn_get_socket(conn), 5) == TH_ERR_OK);

        th_conn_cancel(conn);
        TH_EXPECT(reactor.handle.cancelled);

        th_conn_destroy(conn);
        th_loop_deinit(&loop);
    }
    TH_TEST_CASE_END
}
TH_TEST_END
