#include "th_acceptor.h"
#include "th_system_error.h"
#include "th_test.h"

typedef struct th_fake_handle {
    th_handle base;
    int fd;
    bool timeout_enabled;
    bool cancelled;
    bool destroyed;
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
    (void)op;
    return TH_ERR_OK;
}

static void
th_fake_handle_enable_timeout(void* self, bool enabled)
{
    th_fake_handle* handle = self;
    handle->timeout_enabled = enabled;
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
    th_err create_handle_err;
} th_fake_reactor;

static th_err
th_fake_reactor_create_handle(void* self, th_handle** out, int fd)
{
    th_fake_reactor* reactor = self;
    if (reactor->create_handle_err != TH_ERR_OK)
        return reactor->create_handle_err;
    reactor->handle.base.methods = &th_fake_handle_methods;
    reactor->handle.fd = fd;
    reactor->handle.timeout_enabled = false;
    reactor->handle.cancelled = false;
    reactor->handle.destroyed = false;
    *out = &reactor->handle.base;
    return TH_ERR_OK;
}

static const th_reactor_methods th_fake_reactor_methods = {
    .run = NULL,
    .create_handle = th_fake_reactor_create_handle,
    .destroy = NULL,
};

static void
th_fake_reactor_init(th_fake_reactor* reactor)
{
    reactor->base.methods = &th_fake_reactor_methods;
    reactor->create_handle_err = TH_ERR_OK;
}

typedef struct th_fake_acceptor_ops {
    th_acceptor_ops base;
    th_err open_err;
    int open_fd;
    th_err accept_err;
    int accept_fd;
    int last_fd;
} th_fake_acceptor_ops;

static th_err
th_fake_acceptor_open(void* self, const char* addr, const char* port, int* out_fd)
{
    (void)addr;
    (void)port;
    th_fake_acceptor_ops* ops = self;
    if (ops->open_err != TH_ERR_OK)
        return ops->open_err;
    *out_fd = ops->open_fd;
    return TH_ERR_OK;
}

static th_err
th_fake_acceptor_accept(void* self, int fd, th_address* addr, int* out_fd)
{
    (void)addr;
    th_fake_acceptor_ops* ops = self;
    ops->last_fd = fd;
    if (ops->accept_err != TH_ERR_OK)
        return ops->accept_err;
    *out_fd = ops->accept_fd;
    return TH_ERR_OK;
}

static void
th_fake_acceptor_ops_init(th_fake_acceptor_ops* ops)
{
    ops->base.open = th_fake_acceptor_open;
    ops->base.accept = th_fake_acceptor_accept;
    ops->open_err = TH_ERR_OK;
    ops->open_fd = 9;
    ops->accept_err = TH_ERR_OK;
    ops->accept_fd = -1;
    ops->last_fd = -1;
}

TH_TEST_BEGIN(acceptor)
{
    th_fake_reactor reactor;
    th_fake_reactor_init(&reactor);
    th_loop loop;
    th_loop_init(&loop, &reactor.base);
    th_fake_acceptor_ops ops;
    th_fake_acceptor_ops_init(&ops);
    th_acceptor acceptor;
    th_acceptor_init(&acceptor, &loop, &ops.base);

    TH_TEST_CASE_BEGIN(acceptor_init_has_no_fd)
    {
        TH_EXPECT(th_acceptor_get_fd(&acceptor) == -1);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(acceptor_open_registers_handle_without_timeout)
    {
        TH_EXPECT(th_acceptor_open(&acceptor, "127.0.0.1", "8080") == TH_ERR_OK);
        TH_EXPECT(th_acceptor_get_fd(&acceptor) == 9);
        TH_EXPECT(reactor.handle.timeout_enabled == false);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(acceptor_open_propagates_ops_error)
    {
        ops.open_err = TH_ERR_SYSTEM(TH_EIO);
        TH_EXPECT(th_acceptor_open(&acceptor, "127.0.0.1", "8080") == TH_ERR_SYSTEM(TH_EIO));
        TH_EXPECT(th_acceptor_get_fd(&acceptor) == -1);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(acceptor_open_propagates_reactor_error)
    {
        reactor.create_handle_err = TH_ERR_SYSTEM(TH_EBADF);
        TH_EXPECT(th_acceptor_open(&acceptor, "127.0.0.1", "8080") == TH_ERR_SYSTEM(TH_EBADF));
        TH_EXPECT(th_acceptor_get_fd(&acceptor) == -1);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(acceptor_open_closes_previous_handle)
    {
        ops.open_fd = 1;
        TH_EXPECT(th_acceptor_open(&acceptor, "127.0.0.1", "8080") == TH_ERR_OK);
        ops.open_fd = 2;
        TH_EXPECT(th_acceptor_open(&acceptor, "127.0.0.1", "8080") == TH_ERR_OK);
        TH_EXPECT(th_acceptor_get_fd(&acceptor) == 2);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(acceptor_accept_calls_ops_with_fd)
    {
        TH_EXPECT(th_acceptor_open(&acceptor, "127.0.0.1", "8080") == TH_ERR_OK);
        ops.accept_fd = 42;
        th_address addr;
        int out_fd = -1;
        TH_EXPECT(th_acceptor_accept(&acceptor, &addr, &out_fd) == TH_ERR_OK);
        TH_EXPECT(ops.last_fd == 9);
        TH_EXPECT(out_fd == 42);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(acceptor_accept_propagates_eagain)
    {
        TH_EXPECT(th_acceptor_open(&acceptor, "127.0.0.1", "8080") == TH_ERR_OK);
        ops.accept_err = TH_ERR_SYSTEM(TH_EAGAIN);
        th_address addr;
        int out_fd = -1;
        TH_EXPECT(th_acceptor_accept(&acceptor, &addr, &out_fd) == TH_ERR_SYSTEM(TH_EAGAIN));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(acceptor_cancel_forwards_to_handle)
    {
        TH_EXPECT(th_acceptor_open(&acceptor, "127.0.0.1", "8080") == TH_ERR_OK);
        th_acceptor_cancel(&acceptor);
        TH_EXPECT(reactor.handle.cancelled);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(acceptor_close_destroys_handle_and_clears_fd)
    {
        TH_EXPECT(th_acceptor_open(&acceptor, "127.0.0.1", "8080") == TH_ERR_OK);
        th_acceptor_close(&acceptor);
        TH_EXPECT(reactor.handle.destroyed);
        TH_EXPECT(th_acceptor_get_fd(&acceptor) == -1);
    }
    TH_TEST_CASE_END

    th_acceptor_deinit(&acceptor);
    th_loop_deinit(&loop);
}
TH_TEST_END
