#include "th_mock_syscall.h"
#include "th_tcp_socket.h"
#include "th_test.h"

#include <errno.h>

static th_err last_err = TH_ERR_OK;
static size_t last_result = 0;
static void read_handler(void* data, size_t len, th_err err)
{
    last_result = len;
    last_err = err;
    (void)data;
}

static int
mock_read_eof(void* data, size_t len)
{
    (void)data;
    (void)len;
    return 0;
}

static int
mock_read_bad(void* data, size_t len)
{
    (void)data;
    (void)len;
    return -TH_EIO;
}

static int
mock_write_bad(size_t len)
{

    (void)len;
    return -TH_EIO;
}

TH_TEST_BEGIN(tcp_socket)
{
    TH_TEST_CASE_BEGIN(tcp_socket_init)
    {
        th_tcp_socket socket = {0};
        th_tcp_socket_init(&socket, NULL, NULL);
        th_tcp_socket_deinit(&socket);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(tcp_socket_set_read_good)
    {
        th_context context = {0};
        th_context_init(&context, NULL);
        th_tcp_socket socket = {0};
        th_tcp_socket_init(&socket, &context, NULL);
        th_tcp_socket_set_fd(&socket, 0);
        char buf[512] = {0};
        th_io_handler handler = {0};
        th_io_handler_init(&handler, read_handler, NULL);
        th_tcp_socket_async_read(&socket, buf, sizeof(buf), &handler);
        while (1) {
            if (th_context_poll(&context, -1) != TH_ERR_OK)
                break;
        }
        TH_EXPECT(last_err == TH_ERR_OK);
        TH_EXPECT(last_result == sizeof(buf));
        th_tcp_socket_deinit(&socket);
        th_context_deinit(&context);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(tcp_socket_read_eof)
    {
        th_context context = {0};
        th_context_init(&context, NULL);
        th_tcp_socket socket = {0};
        th_tcp_socket_init(&socket, &context, NULL);
        th_tcp_socket_set_fd(&socket, 0);
        char buf[512] = {0};
        th_io_handler handler = {0};
        th_io_handler_init(&handler, read_handler, NULL);
        th_mock_syscall_get()->read = mock_read_eof;
        th_tcp_socket_async_read(&socket, buf, sizeof(buf), &handler);
        while (1) {
            if (th_context_poll(&context, -1) != TH_ERR_OK)
                break;
        }
        TH_EXPECT(last_err == TH_ERR_EOF);
        th_tcp_socket_deinit(&socket);
        th_context_deinit(&context);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(tcp_socket_read_bad)
    {
        th_context context = {0};
        th_context_init(&context, NULL);
        th_tcp_socket socket = {0};
        th_tcp_socket_init(&socket, &context, NULL);
        th_tcp_socket_set_fd(&socket, 0);
        char buf[512] = {0};
        th_io_handler handler = {0};
        th_io_handler_init(&handler, read_handler, NULL);
        th_mock_syscall_get()->read = mock_read_bad;
        th_tcp_socket_async_read(&socket, buf, sizeof(buf), &handler);
        while (1) {
            if (th_context_poll(&context, -1) != TH_ERR_OK)
                break;
        }
        TH_EXPECT(last_err == TH_ERR_SYSTEM(EIO));
        th_tcp_socket_deinit(&socket);
        th_context_deinit(&context);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(tcp_socket_write_good)
    {
        th_context context = {0};
        th_context_init(&context, NULL);
        th_tcp_socket socket = {0};
        th_tcp_socket_init(&socket, &context, NULL);
        th_tcp_socket_set_fd(&socket, 0);
        char buf[512] = {0};
        th_io_handler handler = {0};
        th_io_handler_init(&handler, read_handler, NULL);
        th_tcp_socket_async_write(&socket, buf, sizeof(buf), &handler);
        while (1) {
            if (th_context_poll(&context, -1) != TH_ERR_OK)
                break;
        }
        TH_EXPECT(last_err == TH_ERR_OK);
        TH_EXPECT(last_result == sizeof(buf));
        th_tcp_socket_deinit(&socket);
        th_context_deinit(&context);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(tcp_socket_write_bad)
    {
        th_context context = {0};
        th_context_init(&context, NULL);
        th_tcp_socket socket = {0};
        th_tcp_socket_init(&socket, &context, NULL);
        th_tcp_socket_set_fd(&socket, 0);
        char buf[512] = {0};
        th_io_handler handler = {0};
        th_io_handler_init(&handler, read_handler, NULL);
        th_mock_syscall_get()->write = mock_write_bad;
        th_tcp_socket_async_write(&socket, buf, sizeof(buf), &handler);
        while (1) {
            if (th_context_poll(&context, -1) != TH_ERR_OK)
                break;
        }
        TH_EXPECT(last_err == TH_ERR_SYSTEM(EIO));
        th_tcp_socket_deinit(&socket);
        th_context_deinit(&context);
    }
    TH_TEST_CASE_END
}
TH_TEST_END
