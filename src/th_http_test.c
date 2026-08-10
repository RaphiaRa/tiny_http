#include "th_conn_tracker.h"
#include "th_fmt.h"
#include "th_http.h"
#include "th_system_error.h"
#include "th_test.h"
#include "th_utility.h"

#include <string.h>

typedef struct th_fake_conn {
    th_conn base;
    th_str request;
    size_t recv_pos;
    char written[2048];
    size_t written_len;
    bool destroyed;

    void (*callback)(void* user_data, size_t size, th_err err);
    void* user_data;
    void* addr;
    size_t len;
} th_fake_conn;

static th_address*
th_fake_conn_get_address(void* self)
{
    (void)self;
    return NULL;
}

static void
th_fake_conn_start(void* self)
{
    (void)self;
}

static void
th_fake_conn_recv(void* self, void* addr, size_t len, bool exact, th_recv_cb callback, void* user_data)
{
    (void)exact;
    th_fake_conn* conn = self;
    TH_ASSERT(conn->callback == NULL);
    conn->addr = addr;
    conn->len = len;
    conn->callback = callback;
    conn->user_data = user_data;
}

static void
th_fake_conn_send(void* self, th_iov* iov, size_t iovcnt, th_file* file, size_t offset, size_t len, th_send_cb callback, void* user_data)
{
    (void)file;
    (void)offset;
    th_fake_conn* conn = self;
    TH_ASSERT(conn->callback == NULL);
    size_t total = 0;
    for (size_t i = 0; i < iovcnt; ++i) {
        memcpy(conn->written + conn->written_len, iov[i].base, iov[i].len);
        conn->written_len += iov[i].len;
        total += iov[i].len;
    }
    conn->addr = NULL;
    conn->len = total + len;
    conn->callback = callback;
    conn->user_data = user_data;
}

static void
th_fake_conn_cancel(void* self)
{
    (void)self;
}

static void
th_fake_conn_destroy(void* self)
{
    th_fake_conn* conn = self;
    conn->destroyed = true;
}

static const th_conn_methods th_fake_conn_methods = {
    .get_address = th_fake_conn_get_address,
    .start = th_fake_conn_start,
    .recv = th_fake_conn_recv,
    .send = th_fake_conn_send,
    .cancel = th_fake_conn_cancel,
    .destroy = th_fake_conn_destroy,
};

static void
th_fake_conn_init(th_fake_conn* conn)
{
    conn->base.methods = &th_fake_conn_methods;
    conn->request = TH_STR("");
    conn->recv_pos = 0;
    conn->written_len = 0;
    conn->destroyed = false;
    conn->callback = NULL;
    conn->user_data = NULL;
    conn->addr = NULL;
    conn->len = 0;
}

static void
th_fake_conn_set_request(th_fake_conn* conn, th_str request)
{
    conn->request = request;
    conn->recv_pos = 0;
}

static void
th_fake_conn_run(th_fake_conn* conn)
{
    TH_ASSERT(conn->callback != NULL);
    void (*callback)(void*, size_t, th_err) = conn->callback;
    void* user_data = conn->user_data;
    conn->callback = NULL;
    conn->user_data = NULL;
    if (conn->addr) {
        size_t remaining = conn->request.len - conn->recv_pos;
        size_t n = TH_MIN(conn->len, remaining);
        memcpy(conn->addr, conn->request.ptr + conn->recv_pos, n);
        conn->recv_pos += n;
        callback(user_data, n, n == 0 ? TH_ERR_EOF : TH_ERR_OK);
    } else {
        callback(user_data, conn->len, TH_ERR_OK);
    }
}

static bool
th_buf_starts_with(const char* buf, size_t buf_len, const char* prefix)
{
    size_t prefix_len = strlen(prefix);
    if (prefix_len > buf_len)
        return false;
    return memcmp(buf, prefix, prefix_len) == 0;
}

static bool
th_buf_ends_with(const char* buf, size_t buf_len, const char* suffix)
{
    size_t suffix_len = strlen(suffix);
    if (suffix_len > buf_len)
        return false;
    return memcmp(buf + buf_len - suffix_len, suffix, suffix_len) == 0;
}

/* Finds a "key: value\r\n" header line anywhere in the headers block and,
 * if expected_value is non-NULL, checks that its value matches exactly. */
static bool
th_buf_has_header(const char* buf, size_t buf_len, const char* key, const char* expected_value)
{
    char prefix[256];
    int prefix_len = snprintf(prefix, sizeof(prefix), "%s: ", key);
    for (size_t i = 0; i + (size_t)prefix_len <= buf_len; ++i) {
        if (memcmp(buf + i, prefix, (size_t)prefix_len) != 0)
            continue;
        size_t value_start = i + (size_t)prefix_len;
        size_t value_end = value_start;
        while (value_end + 1 < buf_len && !(buf[value_end] == '\r' && buf[value_end + 1] == '\n'))
            value_end++;
        if (!expected_value)
            return true;
        size_t expected_len = strlen(expected_value);
        return value_end - value_start == expected_len && memcmp(buf + value_start, expected_value, expected_len) == 0;
    }
    return false;
}

static th_err
th_test_handler(void* user_data, const th_request* req, th_response* resp)
{
    (void)user_data;
    (void)req;
    th_set_body(resp, "Hello, World!");
    return TH_ERR_OK;
}

static th_err
th_test_informational_handler(void* user_data, const th_request* req, th_response* resp)
{
    (void)user_data;
    (void)req;
    (void)resp;
    return TH_ERR_HTTP(100); // Continue
}

static th_err
th_test_system_error_handler(void* user_data, const th_request* req, th_response* resp)
{
    (void)user_data;
    (void)req;
    (void)resp;
    return TH_ERR_SYSTEM(TH_ENOENT);
}

TH_TEST_BEGIN(http)
{
    th_conn_tracker tracker;
    th_conn_tracker_init(&tracker);
    th_router router;
    th_router_init(&router, th_default_allocator_get());
    TH_EXPECT(th_router_add_route(&router, TH_METHOD_GET, TH_STR("/test"), th_test_handler, NULL) == TH_ERR_OK);
    TH_EXPECT(th_router_add_route(&router, TH_METHOD_POST, TH_STR("/test"), th_test_handler, NULL) == TH_ERR_OK);
    TH_EXPECT(th_router_add_route(&router, TH_METHOD_GET, TH_STR("/informational"), th_test_informational_handler, NULL) == TH_ERR_OK);
    TH_EXPECT(th_router_add_route(&router, TH_METHOD_GET, TH_STR("/system-error"), th_test_system_error_handler, NULL) == TH_ERR_OK);
    th_http_upgrader upgrader;
    th_http_upgrader_init(&upgrader, &tracker, &router, NULL, NULL, th_default_allocator_get());
    th_fake_conn conn;
    th_fake_conn_init(&conn);

    TH_TEST_CASE_BEGIN(http_writes_response_for_known_route)
    {
        th_fake_conn_set_request(&conn, TH_STR("GET /test HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n"));

        th_conn_upgrader_upgrade(&upgrader.base, &conn.base);
        while (!conn.destroyed && conn.callback != NULL)
            th_fake_conn_run(&conn);

        TH_EXPECT(th_buf_starts_with(conn.written, conn.written_len, "HTTP/1.1 200 OK\r\n"));
        TH_EXPECT(th_buf_ends_with(conn.written, conn.written_len, "Hello, World!"));
        TH_EXPECT(th_buf_has_header(conn.written, conn.written_len, "Content-Length", "13"));
        TH_EXPECT(th_buf_has_header(conn.written, conn.written_len, "Connection", "close"));
        TH_EXPECT(conn.destroyed);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(http_writes_404_for_unknown_route)
    {
        th_fake_conn_set_request(&conn, TH_STR("GET /nope HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n"));

        th_conn_upgrader_upgrade(&upgrader.base, &conn.base);
        while (!conn.destroyed && conn.callback != NULL)
            th_fake_conn_run(&conn);

        TH_EXPECT(th_buf_starts_with(conn.written, conn.written_len, "HTTP/1.1 404 Not Found\r\n"));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(http_writes_400_for_bad_request)
    {
        th_fake_conn_set_request(&conn, TH_STR("GET \r\n\r\n"));

        th_conn_upgrader_upgrade(&upgrader.base, &conn.base);
        while (!conn.destroyed && conn.callback != NULL)
            th_fake_conn_run(&conn);

        TH_EXPECT(th_buf_starts_with(conn.written, conn.written_len, "HTTP/1.1 400 Bad Request\r\n"));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(http_keeps_connection_alive_for_second_request)
    {
        th_fake_conn_set_request(&conn, TH_STR("GET /test HTTP/1.1\r\nHost: example.com\r\n\r\n"));

        th_conn_upgrader_upgrade(&upgrader.base, &conn.base);
        // Drive the first request/response cycle: recv(s) until the
        // response is sent back, stopping right after that send completes
        // and before th_http_restart's next recv is run.
        while (conn.written_len == 0)
            th_fake_conn_run(&conn);
        TH_EXPECT(!conn.destroyed);
        TH_EXPECT(th_buf_starts_with(conn.written, conn.written_len, "HTTP/1.1 200 OK\r\n"));
        TH_EXPECT(th_buf_has_header(conn.written, conn.written_len, "Connection", "keep-alive"));
        th_fake_conn_run(&conn); // completes the send, triggers th_http_restart's recv
        TH_EXPECT(!conn.destroyed);
        TH_EXPECT(conn.callback != NULL);

        // Second request on the same (still alive) connection.
        conn.written_len = 0;
        th_fake_conn_set_request(&conn, TH_STR("GET /test HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n"));
        while (!conn.destroyed && conn.callback != NULL)
            th_fake_conn_run(&conn);

        TH_EXPECT(conn.destroyed);
        TH_EXPECT(th_buf_starts_with(conn.written, conn.written_len, "HTTP/1.1 200 OK\r\n"));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(http_handles_partial_header)
    {
        //"GET /test HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n");
        th_fake_conn_set_request(&conn, TH_STR("GET /test HTTP/1.1\r\nHost: ex"));
        th_conn_upgrader_upgrade(&upgrader.base, &conn.base);
        th_fake_conn_run(&conn);
        TH_EXPECT(!conn.destroyed);
        TH_EXPECT(conn.written_len == 0); // still waiting on the rest of the header

        // remaining bytes
        th_fake_conn_set_request(&conn, TH_STR("ample.com\r\nConnection: close\r\n\r\n"));
        while (!conn.destroyed && conn.callback != NULL)
            th_fake_conn_run(&conn);

        TH_EXPECT(th_buf_starts_with(conn.written, conn.written_len, "HTTP/1.1 200 OK\r\n"));
        TH_EXPECT(th_buf_ends_with(conn.written, conn.written_len, "Hello, World!"));
        TH_EXPECT(conn.destroyed);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(http_handles_partial_body)
    {
        // POST /test HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\nContent-Length: 11\r\n\r\nHello, World!
        th_fake_conn_set_request(&conn, TH_STR("POST /test HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\nContent-Length: 11\r\n\r\nHello"));
        th_conn_upgrader_upgrade(&upgrader.base, &conn.base);
        th_fake_conn_run(&conn);
        TH_EXPECT(!conn.destroyed);
        TH_EXPECT(conn.written_len == 0); // still waiting on the rest of the body

        // Remaining bytes
        th_fake_conn_set_request(&conn, TH_STR(", World!"));
        while (!conn.destroyed && conn.callback != NULL)
            th_fake_conn_run(&conn);

        TH_EXPECT(th_buf_starts_with(conn.written, conn.written_len, "HTTP/1.1 200 OK\r\n"));
        TH_EXPECT(th_buf_ends_with(conn.written, conn.written_len, "Hello, World!"));
        TH_EXPECT(conn.destroyed);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(http_rejects_header_too_large)
    {
        // Header never terminates and keeps growing past
        // TH_CONFIG_LARGE_HEADER_LEN, so it's rejected outright rather
        // than resized indefinitely.
        char request[TH_CONFIG_LARGE_HEADER_LEN + 256];
        size_t pos = 0;
        pos += th_fmt_str_append(request, pos, sizeof(request), "GET /test HTTP/1.1\r\n");
        while (pos + 32 < sizeof(request)) {
            pos += th_fmt_str_append(request, pos, sizeof(request), "X-Pad: aaaaaaaaaaaaaaaaaaaaaaaa\r\n");
        }
        th_fake_conn_set_request(&conn, th_str_make(request, pos));

        th_conn_upgrader_upgrade(&upgrader.base, &conn.base);
        while (!conn.destroyed && conn.callback != NULL)
            th_fake_conn_run(&conn);

        TH_EXPECT(th_buf_starts_with(conn.written, conn.written_len, "HTTP/1.1 431 Request Header Fields Too Large\r\n"));
        TH_EXPECT(conn.destroyed);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(http_rejects_body_too_large)
    {
        char request[256];
        size_t pos = 0;
        pos += th_fmt_str_append(request, pos, sizeof(request), "POST /test HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\nContent-Length: ");
        char content_len[32];
        pos += th_fmt_str_append(request, pos, sizeof(request), th_fmt_uint_to_str(content_len, sizeof(content_len), TH_MAX_BODY_LEN + 1));
        pos += th_fmt_str_append(request, pos, sizeof(request), "\r\n\r\n");
        th_fake_conn_set_request(&conn, th_str_make(request, pos));

        th_conn_upgrader_upgrade(&upgrader.base, &conn.base);
        while (!conn.destroyed && conn.callback != NULL)
            th_fake_conn_run(&conn);

        TH_EXPECT(th_buf_starts_with(conn.written, conn.written_len, "HTTP/1.1 413 Payload Too Large\r\n"));
        TH_EXPECT(conn.destroyed);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(http_accepts_large_body_growing_internal_buffer)
    {
        // Body alone is bigger than TH_CONFIG_SMALL_HEADER_LEN (the
        // initial buffer size), but well within TH_MAX_BODY_LEN, so the
        // request is accepted and th_buf_vec_resize's growth path runs.
        size_t body_len = TH_CONFIG_SMALL_HEADER_LEN + 1000;
        char request[TH_CONFIG_SMALL_HEADER_LEN + 1200];
        size_t pos = 0;
        pos += th_fmt_str_append(request, pos, sizeof(request), "POST /test HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\nContent-Length: ");
        char content_len[32];
        pos += th_fmt_str_append(request, pos, sizeof(request), th_fmt_uint_to_str(content_len, sizeof(content_len), (unsigned int)body_len));
        pos += th_fmt_str_append(request, pos, sizeof(request), "\r\n\r\n");
        for (size_t i = 0; i < body_len; ++i)
            request[pos++] = 'a';
        th_fake_conn_set_request(&conn, th_str_make(request, pos));

        th_conn_upgrader_upgrade(&upgrader.base, &conn.base);
        while (!conn.destroyed && conn.callback != NULL)
            th_fake_conn_run(&conn);

        TH_EXPECT(th_buf_starts_with(conn.written, conn.written_len, "HTTP/1.1 200 OK\r\n"));
        TH_EXPECT(th_buf_ends_with(conn.written, conn.written_len, "Hello, World!"));
        TH_EXPECT(conn.destroyed);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(http_rejects_too_many_connections)
    {
        // Rejected outright at upgrade time, before any request is read.
        tracker.count = TH_CONFIG_MAX_CONNECTIONS + 1;

        th_conn_upgrader_upgrade(&upgrader.base, &conn.base);
        while (!conn.destroyed && conn.callback != NULL)
            th_fake_conn_run(&conn);

        TH_EXPECT(th_buf_starts_with(conn.written, conn.written_len, "HTTP/1.1 503 Service Unavailable\r\n"));
        TH_EXPECT(conn.destroyed);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(http_head_request_writes_headers_without_body)
    {
        th_fake_conn_set_request(&conn, TH_STR("HEAD /test HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n"));

        th_conn_upgrader_upgrade(&upgrader.base, &conn.base);
        while (!conn.destroyed && conn.callback != NULL)
            th_fake_conn_run(&conn);

        TH_EXPECT(th_buf_starts_with(conn.written, conn.written_len, "HTTP/1.1 200 OK\r\n"));
        TH_EXPECT(th_buf_has_header(conn.written, conn.written_len, "Content-Length", "13")); // matches GET's body length
        TH_EXPECT(!th_buf_ends_with(conn.written, conn.written_len, "Hello, World!"));        // but body itself is omitted
        TH_EXPECT(conn.destroyed);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(http_handles_options_for_known_route)
    {
        th_fake_conn_set_request(&conn, TH_STR("OPTIONS /test HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n"));

        th_conn_upgrader_upgrade(&upgrader.base, &conn.base);
        while (!conn.destroyed && conn.callback != NULL)
            th_fake_conn_run(&conn);

        TH_EXPECT(th_buf_starts_with(conn.written, conn.written_len, "HTTP/1.1 200 OK\r\n"));
        TH_EXPECT(th_buf_has_header(conn.written, conn.written_len, "Allow", "OPTIONS, GET, HEAD, POST"));
        TH_EXPECT(th_buf_has_header(conn.written, conn.written_len, "Content-Type", "text/plain"));
        TH_EXPECT(conn.destroyed);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(http_handles_options_wildcard)
    {
        th_fake_conn_set_request(&conn, TH_STR("OPTIONS * HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n"));

        th_conn_upgrader_upgrade(&upgrader.base, &conn.base);
        while (!conn.destroyed && conn.callback != NULL)
            th_fake_conn_run(&conn);

        TH_EXPECT(th_buf_starts_with(conn.written, conn.written_len, "HTTP/1.1 200 OK\r\n"));
        TH_EXPECT(th_buf_has_header(conn.written, conn.written_len, "Allow", "OPTIONS, GET, HEAD, POST, PUT, DELETE, PATCH"));
        TH_EXPECT(conn.destroyed);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(http_writes_informational_response_for_1_1)
    {
        th_fake_conn_set_request(&conn, TH_STR("GET /informational HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n"));

        th_conn_upgrader_upgrade(&upgrader.base, &conn.base);
        while (!conn.destroyed && conn.callback != NULL)
            th_fake_conn_run(&conn);

        TH_EXPECT(th_buf_starts_with(conn.written, conn.written_len, "HTTP/1.1 100 "));
        TH_EXPECT(conn.destroyed);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(http_rejects_informational_response_for_1_0)
    {
        // HTTP/1.0 clients can't handle 1xx responses, so this is
        // downgraded to a 400 Bad Request instead.
        th_fake_conn_set_request(&conn, TH_STR("GET /informational HTTP/1.0\r\nHost: example.com\r\nConnection: close\r\n\r\n"));

        th_conn_upgrader_upgrade(&upgrader.base, &conn.base);
        while (!conn.destroyed && conn.callback != NULL)
            th_fake_conn_run(&conn);

        TH_EXPECT(th_buf_starts_with(conn.written, conn.written_len, "HTTP/1.1 400 Bad Request\r\n"));
        TH_EXPECT(conn.destroyed);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(http_maps_unrelated_system_error_to_404)
    {
        // A handler returning a plain system error (not an HTTP error) gets
        // translated by th_http_error, e.g. ENOENT maps to 404.
        th_fake_conn_set_request(&conn, TH_STR("GET /system-error HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n"));

        th_conn_upgrader_upgrade(&upgrader.base, &conn.base);
        while (!conn.destroyed && conn.callback != NULL)
            th_fake_conn_run(&conn);

        TH_EXPECT(th_buf_starts_with(conn.written, conn.written_len, "HTTP/1.1 404 Not Found\r\n"));
        TH_EXPECT(conn.destroyed);
    }
    TH_TEST_CASE_END

    th_router_deinit(&router);
    th_conn_tracker_deinit(&tracker);
}
TH_TEST_END
