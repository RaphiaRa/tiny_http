#include "th_accept.h"
#include "th_system_error.h"
#include "th_test.h"

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

typedef struct th_fake_acceptor_ops {
    th_acceptor_ops base;
    int open_fd;
    th_err accept_err; /* returned once, then reset to TH_ERR_OK */
    int accept_fd;
} th_fake_acceptor_ops;

static th_err
th_fake_acceptor_open(void* self, const char* addr, const char* port, int* out_fd)
{
    (void)addr;
    (void)port;
    th_fake_acceptor_ops* ops = self;
    *out_fd = ops->open_fd;
    return TH_ERR_OK;
}

static th_err
th_fake_acceptor_accept(void* self, int fd, th_address* addr, int* out_fd)
{
    (void)fd;
    (void)addr;
    th_fake_acceptor_ops* ops = self;
    if (ops->accept_err != TH_ERR_OK) {
        th_err err = ops->accept_err;
        ops->accept_err = TH_ERR_OK;
        return err;
    }
    *out_fd = ops->accept_fd;
    return TH_ERR_OK;
}

static void
th_fake_acceptor_ops_init(th_fake_acceptor_ops* ops)
{
    ops->base.open = th_fake_acceptor_open;
    ops->base.accept = th_fake_acceptor_accept;
    ops->open_fd = 9;
    ops->accept_err = TH_ERR_OK;
    ops->accept_fd = -1;
}

TH_INLINE(void)
th_fake_acceptor_ops_reset(th_fake_acceptor_ops* ops)
{
    ops->accept_err = TH_ERR_OK;
    ops->accept_fd = -1;
}

typedef struct th_recorded_result {
    bool called;
    th_err err;
} th_recorded_result;

static void
th_recorded_result_cb(void* user_data, th_err err)
{
    th_recorded_result* result = user_data;
    result->called = true;
    result->err = err;
}

static void
th_recorded_result_init(th_recorded_result* result)
{
    result->called = false;
    result->err = TH_ERR_OK;
}

TH_TEST_BEGIN(accept)
{
    th_fake_reactor reactor;
    th_fake_reactor_init(&reactor);
    th_loop loop;
    th_loop_init(&loop, &reactor.base);
    th_fake_acceptor_ops ops;
    th_fake_acceptor_ops_init(&ops);
    th_acceptor acceptor;
    th_acceptor_init(&acceptor, &loop, &ops.base);
    th_acceptor_open(&acceptor, "127.0.0.1", "8080");

    TH_TEST_CASE_BEGIN(accept_completes_with_new_fd)
    {
        th_fake_acceptor_ops_reset(&ops);
        ops.accept_fd = 42;

        th_socket socket;
        th_socket_init(&socket, &loop, NULL);
        th_address addr;
        th_recorded_result result;
        th_recorded_result_init(&result);
        th_accept_op op;
        th_accept_op_init(&op, &acceptor, &addr, &socket, th_recorded_result_cb, &result);
        th_op_perform(&op.base);
        th_loop_run(&loop);

        TH_EXPECT(result.called);
        TH_EXPECT(result.err == TH_ERR_OK);
        TH_EXPECT(th_socket_get_fd(&socket) == 42);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(accept_eagain_submits_and_retries)
    {
        th_fake_acceptor_ops_reset(&ops);
        ops.accept_err = TH_ERR_SYSTEM(TH_EAGAIN);
        ops.accept_fd = 7;

        th_socket socket;
        th_socket_init(&socket, &loop, NULL);
        th_address addr;
        th_recorded_result result;
        th_recorded_result_init(&result);
        th_accept_op op;
        th_accept_op_init(&op, &acceptor, &addr, &socket, th_recorded_result_cb, &result);
        th_op_perform(&op.base);
        th_loop_run(&loop);

        TH_EXPECT(result.called);
        TH_EXPECT(result.err == TH_ERR_OK);
        TH_EXPECT(th_socket_get_fd(&socket) == 7);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(accept_error_completes_with_error)
    {
        th_fake_acceptor_ops_reset(&ops);
        ops.accept_err = TH_ERR_SYSTEM(TH_EIO);

        th_socket socket;
        th_socket_init(&socket, &loop, NULL);
        th_address addr;
        th_recorded_result result;
        th_recorded_result_init(&result);
        th_accept_op op;
        th_accept_op_init(&op, &acceptor, &addr, &socket, th_recorded_result_cb, &result);
        th_op_perform(&op.base);
        th_loop_run(&loop);

        TH_EXPECT(result.called);
        TH_EXPECT(result.err == TH_ERR_SYSTEM(TH_EIO));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(accept_abort_completes_with_given_error)
    {
        th_fake_acceptor_ops_reset(&ops);

        th_socket socket;
        th_socket_init(&socket, &loop, NULL);
        th_address addr;
        th_recorded_result result;
        th_recorded_result_init(&result);
        th_accept_op op;
        th_accept_op_init(&op, &acceptor, &addr, &socket, th_recorded_result_cb, &result);
        th_op_abort(&op.base, TH_ERR_SYSTEM(TH_ECANCELED));
        th_loop_run(&loop);

        TH_EXPECT(result.called);
        TH_EXPECT(result.err == TH_ERR_SYSTEM(TH_ECANCELED));
    }
    TH_TEST_CASE_END

    th_acceptor_deinit(&acceptor);
    th_loop_deinit(&loop);
}
TH_TEST_END
