#include "th_context.h"
#include "th_mock_service.h"
#include "th_mock_syscall.h"
#include "th_response.h"
#include "th_tcp_socket.h"
#include "th_test.h"

#include <string.h>

static th_err last_err = TH_ERR_OK;
static void write_handler(void* data, size_t len, th_err err)
{
    (void)data;
    (void)len;
    last_err = err;
}

#define TH_SETUP_BASIC(context, response, socket)                      \
    th_context context = {0};                                          \
    th_context_init(&context, NULL);                                   \
    th_fcache fcache = {0};                                            \
    th_fcache_init(&fcache, th_default_allocator_get());               \
    th_response response = {0};                                        \
    th_response_init(&response, &fcache, th_default_allocator_get());  \
    th_tcp_socket socket = {0};                                        \
    th_tcp_socket_init(&socket, &context, th_default_allocator_get()); \
    th_tcp_socket_set_fd(&socket, 0);                                  \
    th_io_handler handler = {0};                                       \
    th_io_handler_init(&handler, write_handler, NULL);

#define TH_SHUTDOWN_BASIC(context, response, socket) \
    th_tcp_socket_set_fd(&socket, -1);               \
    th_tcp_socket_close(&socket);                    \
    th_response_deinit(&response);                   \
    th_fcache_deinit(&fcache);                       \
    th_context_deinit(&context);

TH_TEST_BEGIN(response)
{
    TH_TEST_CASE_BEGIN(response_create_and_destroy)
    {
        TH_SETUP_BASIC(context, response, socket);
        TH_SHUTDOWN_BASIC(context, response, socket);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(response_write_without_content)
    {
        TH_SETUP_BASIC(context, response, socket);
        th_response_async_write(&response, &socket.base, &handler);
        while (1) {
            if (th_context_poll(&context, -1) != TH_ERR_OK)
                break;
        }
        TH_EXPECT(last_err == TH_ERR_OK);
        TH_SHUTDOWN_BASIC(context, response, socket);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(response_write_with_content)
    {
        TH_SETUP_BASIC(context, response, socket);
        th_set_body(&response, "Hello, World!");
        th_response_async_write(&response, &socket.base, &handler);
        while (1) {
            if (th_context_poll(&context, -1) != TH_ERR_OK)
                break;
        }
        TH_EXPECT(last_err == TH_ERR_OK);
        TH_SHUTDOWN_BASIC(context, response, socket);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(response_write_with_content_and_header)
    {
        TH_SETUP_BASIC(context, response, socket);
        th_set_body(&response, "Hello, World!");
        th_add_header(&response, "Connection", "close");
        th_add_header(&response, "Content-Type", "text/plain");
        th_response_async_write(&response, &socket.base, &handler);
        while (1) {
            if (th_context_poll(&context, -1) != TH_ERR_OK)
                break;
        }
        TH_EXPECT(last_err == TH_ERR_OK);
        TH_SHUTDOWN_BASIC(context, response, socket);
    }
    TH_TEST_CASE_END
}
TH_TEST_END
