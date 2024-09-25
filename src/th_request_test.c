#include "th_context.h"
#include "th_mock_service.h"
#include "th_mock_syscall.h"
#include "th_request.h"
#include "th_tcp_socket.h"
#include "th_test.h"

#include <string.h>

static th_err last_err = TH_ERR_OK;
static void read_handler(void* data, size_t len, th_err err)
{
    (void)data;
    (void)len;
    last_err = err;
}

typedef struct mock_data {
    const char* data;
    size_t len;
    size_t pos;
} mock_data;

static mock_data* mock_data_get(void)
{
    static mock_data data = {0};
    return &data;
}

static void mock_data_set(const char* data)
{
    mock_data* mock = mock_data_get();
    mock->data = data;
    mock->len = strlen(data);
    mock->pos = 0;
}

static int mock_read_good(void* data, size_t len)
{
    mock_data* mock = mock_data_get();
    size_t avail = mock->len - mock->pos;
    size_t read = len < avail ? len : avail;
    memcpy(data, mock->data + mock->pos, read);
    mock->pos += read;
    return read;
}

#define TH_SETUP_BASIC(context, request, socket)                       \
    th_context context = {0};                                          \
    th_context_init(&context, NULL);                                   \
    th_request request = {0};                                          \
    th_request_init(&request, NULL);                                   \
    th_tcp_socket socket = {0};                                        \
    th_tcp_socket_init(&socket, &context, th_default_allocator_get()); \
    th_tcp_socket_set_fd(&socket, 0);                                  \
    th_io_handler handler = {0};                                       \
    th_io_handler_init(&handler, read_handler, NULL);

#define TH_SHUTDOWN_BASIC(context, request, socket) \
    th_tcp_socket_set_fd(&socket, -1);              \
    th_tcp_socket_close(&socket);                   \
    th_request_deinit(&request);                    \
    th_context_deinit(&context);

TH_TEST_BEGIN(request)
{
    th_mock_syscall_get()->read = mock_read_good;
    TH_TEST_CASE_BEGIN(request_create_and_destroy)
    {
        th_request request = {0};
        th_request_init(&request, NULL);
        th_request_deinit(&request);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(request_read)
    {
        TH_SETUP_BASIC(context, request, socket);
        mock_data_set("GET / HTTP/1.1\r\nHost: example.com\r\n\r\n");
        th_request_async_read(&socket.base, th_default_allocator_get(), &request, &handler);
        while (1) {
            if (th_context_poll(&context, -1) != TH_ERR_OK)
                break;
        }
        TH_EXPECT(last_err == TH_ERR_OK);
        TH_EXPECT(request.method == TH_METHOD_GET);
        TH_EXPECT(strcmp(request.uri_path, "/") == 0);
        TH_EXPECT(request.minor_version == 1);
        TH_SHUTDOWN_BASIC(context, request, socket);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(request_read_content)
    {
        TH_SETUP_BASIC(context, request, socket);
        mock_data_set("GET / HTTP/1.1\r\nHost: example.com\r\nContent-Length: 5\r\n\r\nhello");
        th_request_async_read(&socket.base, th_default_allocator_get(), &request, &handler);
        while (1) {
            if (th_context_poll(&context, -1) != TH_ERR_OK)
                break;
        }
        TH_EXPECT(last_err == TH_ERR_OK);
        TH_EXPECT(request.method == TH_METHOD_GET);
        TH_EXPECT(strcmp(request.uri_path, "/") == 0);
        TH_EXPECT(request.minor_version == 1);
        th_buffer buffer = th_get_body(&request);
        TH_EXPECT(strncmp(buffer.ptr, "hello", buffer.len) == 0);
        TH_SHUTDOWN_BASIC(context, request, socket);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(request_path_query)
    {
        TH_SETUP_BASIC(context, request, socket);
        mock_data_set("GET /test?key1=value1&key2=value2&key3=value3 HTTP/1.1\r\nHost: example.com\r\n\r\n");
        th_request_async_read(&socket.base, th_default_allocator_get(), &request, &handler);
        while (1) {
            if (th_context_poll(&context, -1) != TH_ERR_OK)
                break;
        }
        TH_EXPECT(last_err == TH_ERR_OK);
        TH_EXPECT(request.method == TH_METHOD_GET);
        TH_EXPECT(strcmp(request.uri_path, "/test") == 0);
        TH_EXPECT(strcmp(request.uri_query, "key1=value1&key2=value2&key3=value3") == 0);
        th_map* params = th_get_query_params(&request);
        TH_EXPECT(params);
        th_map_iter param = th_map_find(params, "key1");
        TH_EXPECT(param && strcmp(param->value, "value1") == 0);
        param = th_map_find(params, "key2");
        TH_EXPECT(param && strcmp(param->value, "value2") == 0);
        param = th_map_find(params, "key3");
        TH_EXPECT(param && strcmp(param->value, "value3") == 0);
        param = th_map_find(params, "key4");
        TH_EXPECT(param == NULL);
        TH_SHUTDOWN_BASIC(context, request, socket);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(request_cookies)
    {
        TH_SETUP_BASIC(context, request, socket);
        mock_data_set("GET /test HTTP/1.1\r\nHost: example.com\r\nCookie: key1=value1; key2=value2; key3=value3\r\n\r\n");
        th_request_async_read(&socket.base, th_default_allocator_get(), &request, &handler);
        while (1) {
            if (th_context_poll(&context, -1) != TH_ERR_OK)
                break;
        }
        TH_EXPECT(last_err == TH_ERR_OK);
        TH_EXPECT(request.method == TH_METHOD_GET);
        TH_EXPECT(strcmp(request.uri_path, "/test") == 0);
        th_map* cookies = th_get_cookies(&request);
        th_map_iter iter = th_map_find(cookies, "key1");
        TH_EXPECT(iter && strcmp(iter->value, "value1") == 0);
        iter = th_map_find(cookies, "key2");
        TH_EXPECT(iter && strcmp(iter->value, "value2") == 0);
        iter = th_map_find(cookies, "key3");
        TH_EXPECT(iter && strcmp(iter->value, "value3") == 0);
        iter = th_map_find(cookies, "key4");
        TH_EXPECT(iter == NULL);
        TH_SHUTDOWN_BASIC(context, request, socket);
    }
    TH_TEST_CASE_END
}
TH_TEST_END
