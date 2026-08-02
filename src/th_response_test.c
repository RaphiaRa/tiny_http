#include "th_response.h"
#include "th_test.h"

#include <string.h>

typedef struct th_fake_conn {
    th_conn base;
    char written[1024];
    size_t written_len;
    bool sent_file;
    size_t file_offset;
    size_t file_len;
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
th_fake_conn_send(void* self, th_iov* iov, size_t iovcnt, th_file* file, size_t offset, size_t len, th_send_cb callback, void* user_data)
{
    th_fake_conn* conn = self;
    size_t total = 0;
    for (size_t i = 0; i < iovcnt; ++i) {
        memcpy(conn->written + conn->written_len, iov[i].base, iov[i].len);
        conn->written_len += iov[i].len;
        total += iov[i].len;
    }
    conn->sent_file = file != NULL;
    conn->file_offset = offset;
    conn->file_len = len;
    callback(user_data, total + len, TH_ERR_OK);
}

static void
th_fake_conn_cancel(void* self)
{
    (void)self;
}

static void
th_fake_conn_destroy(void* self)
{
    (void)self;
}

static const th_conn_methods th_fake_conn_methods = {
    .get_address = th_fake_conn_get_address,
    .start = th_fake_conn_start,
    .recv = NULL,
    .send = th_fake_conn_send,
    .cancel = th_fake_conn_cancel,
    .destroy = th_fake_conn_destroy,
};

static void
th_fake_conn_init(th_fake_conn* conn)
{
    conn->base.methods = &th_fake_conn_methods;
    conn->written_len = 0;
    conn->sent_file = false;
    conn->file_offset = 0;
    conn->file_len = 0;
}

/* Plain byte-substring search, rather than memmem: ASan's memmem
 * interceptor in this environment spuriously returns NULL for a needle
 * that is genuinely present (reproduced independent of this codebase). */
static bool
th_buf_contains(const char* haystack, size_t haystack_len, const char* needle)
{
    size_t needle_len = strlen(needle);
    if (needle_len > haystack_len)
        return false;
    for (size_t i = 0; i + needle_len <= haystack_len; ++i) {
        if (memcmp(haystack + i, needle, needle_len) == 0)
            return true;
    }
    return false;
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

TH_TEST_BEGIN(response)
{
    th_dir_mgr dir_mgr;
    th_dir_mgr_init(&dir_mgr, th_default_allocator_get());
    th_fcache fcache;
    th_fcache_init(&fcache, th_default_allocator_get());
    th_response response;
    th_response_init(&response, &dir_mgr, &fcache, th_default_allocator_get());
    th_fake_conn conn;
    th_fake_conn_init(&conn);

    TH_TEST_CASE_BEGIN(response_write_without_content)
    {
        th_recorded_result result;
        th_recorded_result_init(&result);
        th_response_async_write(&response, &conn.base, th_recorded_result_cb, &result);

        TH_EXPECT(result.called);
        TH_EXPECT(result.err == TH_ERR_OK);
        TH_EXPECT(conn.sent_file == false);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(response_write_with_content)
    {
        th_set_body(&response, "Hello, World!");

        th_recorded_result result;
        th_recorded_result_init(&result);
        th_response_async_write(&response, &conn.base, th_recorded_result_cb, &result);

        TH_EXPECT(result.called);
        TH_EXPECT(result.err == TH_ERR_OK);
        TH_EXPECT(th_buf_contains(conn.written, conn.written_len, "Hello, World!"));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(response_write_with_content_and_header)
    {
        th_set_body(&response, "Hello, World!");
        th_add_header(&response, "Connection", "close");
        th_add_header(&response, "Content-Type", "text/plain");

        th_recorded_result result;
        th_recorded_result_init(&result);
        th_response_async_write(&response, &conn.base, th_recorded_result_cb, &result);

        TH_EXPECT(result.called);
        TH_EXPECT(result.err == TH_ERR_OK);
        TH_EXPECT(th_buf_contains(conn.written, conn.written_len, "Connection: close"));
        TH_EXPECT(th_buf_contains(conn.written, conn.written_len, "Content-Type: text/plain"));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(response_only_headers_skips_body)
    {
        th_set_body(&response, "Hello, World!");
        response.only_headers = true;

        th_recorded_result result;
        th_recorded_result_init(&result);
        th_response_async_write(&response, &conn.base, th_recorded_result_cb, &result);

        TH_EXPECT(result.called);
        TH_EXPECT(result.err == TH_ERR_OK);
        TH_EXPECT(!th_buf_contains(conn.written, conn.written_len, "Hello, World!"));
    }
    TH_TEST_CASE_END

    th_response_deinit(&response);
    th_fcache_deinit(&fcache);
    th_dir_mgr_deinit(&dir_mgr);
}
TH_TEST_END
