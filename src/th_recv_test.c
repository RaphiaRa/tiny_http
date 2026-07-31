#include "th_recv.h"
#include "th_system_error.h"
#include "th_test.h"

#include <string.h>

typedef struct th_fake_handle {
    th_handle base;
    int fd;
} th_fake_handle;

/* Simulates a reactor that is always immediately ready, driving the
 * op's retry loop synchronously instead of waiting for a real event. */
static th_err
th_fake_handle_submit(void* self, th_op* op)
{
    (void)self;
    th_op_perform(op);
    return TH_ERR_OK;
}

static int
th_fake_handle_get_fd(const void* self)
{
    const th_fake_handle* handle = self;
    return handle->fd;
}

static void
th_fake_handle_enable_timeout(void* self, bool enabled)
{
    (void)self;
    (void)enabled;
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

typedef struct th_fake_socket_ops {
    th_socket_ops base;
    const char* data;
    size_t data_len;
    size_t chunk_len; /* max bytes returned per call; 0 = unlimited */
    th_err err;       /* returned once, then reset to TH_ERR_OK */
} th_fake_socket_ops;

static th_err
th_fake_recv(void* self, int fd, void* addr, size_t len, size_t* result)
{
    (void)fd;
    th_fake_socket_ops* ops = self;
    if (ops->err != TH_ERR_OK) {
        th_err err = ops->err;
        ops->err = TH_ERR_OK;
        return err;
    }
    size_t avail = ops->data_len;
    size_t n = len < avail ? len : avail;
    if (ops->chunk_len != 0 && n > ops->chunk_len)
        n = ops->chunk_len;
    memcpy(addr, ops->data, n);
    ops->data += n;
    ops->data_len -= n;
    *result = n;
    return TH_ERR_OK;
}

static void
th_fake_socket_ops_init(th_fake_socket_ops* ops)
{
    ops->base.send = NULL;
    ops->base.sendvec = NULL;
    ops->base.recv = th_fake_recv;
    ops->data = NULL;
    ops->data_len = 0;
    ops->chunk_len = 0;
    ops->err = TH_ERR_OK;
}

TH_INLINE(void)
th_fake_socket_ops_set_data(th_fake_socket_ops* ops, const char* data, size_t data_len)
{
    ops->data = data;
    ops->data_len = data_len;
}

typedef struct th_recorded_result {
    bool called;
    size_t result;
    th_err err;
} th_recorded_result;

static void
th_recorded_result_cb(void* user_data, size_t size, th_err err)
{
    th_recorded_result* result = user_data;
    result->called = true;
    result->result = size;
    result->err = err;
}

static void
th_recorded_result_init(th_recorded_result* result)
{
    result->called = false;
    result->result = 0;
    result->err = TH_ERR_OK;
}

TH_TEST_BEGIN(recv)
{
    th_fake_reactor reactor;
    th_fake_reactor_init(&reactor);
    th_loop loop;
    th_loop_init(&loop, &reactor.base);
    th_fake_socket_ops ops;
    th_fake_socket_ops_init(&ops);
    th_socket socket;
    th_socket_init(&socket, &loop, &ops.base);
    th_socket_set_fd(&socket, 5);

    TH_TEST_CASE_BEGIN(recv_partial_completes_immediately_when_not_exact)
    {
        th_fake_socket_ops_set_data(&ops, "hello world", 11);

        char buf[32] = {0};
        th_recorded_result result;
        th_recorded_result_init(&result);
        th_recv_op op;
        th_recv_op_init(&op, &socket, buf, sizeof(buf), false, th_recorded_result_cb, &result);
        th_op_perform(&op.base);
        th_loop_run(&loop);

        TH_EXPECT(result.called);
        TH_EXPECT(result.err == TH_ERR_OK);
        TH_EXPECT(result.result == 11);
        TH_EXPECT(memcmp(buf, "hello world", 11) == 0);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(recv_exact_retries_until_full_length_read)
    {
        th_fake_socket_ops_set_data(&ops, "hello world", 11);
        ops.chunk_len = 4;

        char buf[11] = {0};
        th_recorded_result result;
        th_recorded_result_init(&result);
        th_recv_op op;
        th_recv_op_init(&op, &socket, buf, sizeof(buf), true, th_recorded_result_cb, &result);
        th_op_perform(&op.base);
        th_loop_run(&loop);

        TH_EXPECT(result.called);
        TH_EXPECT(result.err == TH_ERR_OK);
        TH_EXPECT(result.result == 11);
        TH_EXPECT(memcmp(buf, "hello world", 11) == 0);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(recv_eagain_submits_and_retries)
    {
        th_fake_socket_ops_set_data(&ops, "hi", 2);
        ops.err = TH_ERR_SYSTEM(TH_EAGAIN);

        char buf[2] = {0};
        th_recorded_result result;
        th_recorded_result_init(&result);
        th_recv_op op;
        th_recv_op_init(&op, &socket, buf, sizeof(buf), false, th_recorded_result_cb, &result);
        th_op_perform(&op.base);
        th_loop_run(&loop);

        TH_EXPECT(result.called);
        TH_EXPECT(result.err == TH_ERR_OK);
        TH_EXPECT(result.result == 2);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(recv_eof_completes_with_error)
    {
        th_fake_socket_ops_set_data(&ops, "", 0);
        ops.err = TH_ERR_EOF;

        char buf[4] = {0};
        th_recorded_result result;
        th_recorded_result_init(&result);
        th_recv_op op;
        th_recv_op_init(&op, &socket, buf, sizeof(buf), false, th_recorded_result_cb, &result);
        th_op_perform(&op.base);
        th_loop_run(&loop);

        TH_EXPECT(result.called);
        TH_EXPECT(result.err == TH_ERR_EOF);
        TH_EXPECT(result.result == 0);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(recv_abort_completes_with_given_error)
    {
        th_fake_socket_ops_set_data(&ops, "x", 1);

        char buf[4] = {0};
        th_recorded_result result;
        th_recorded_result_init(&result);
        th_recv_op op;
        th_recv_op_init(&op, &socket, buf, sizeof(buf), false, th_recorded_result_cb, &result);
        th_op_abort(&op.base, TH_ERR_SYSTEM(TH_ECANCELED));
        th_loop_run(&loop);

        TH_EXPECT(result.called);
        TH_EXPECT(result.err == TH_ERR_SYSTEM(TH_ECANCELED));
    }
    TH_TEST_CASE_END

    th_loop_deinit(&loop);
    th_socket_deinit(&socket);
}
TH_TEST_END
