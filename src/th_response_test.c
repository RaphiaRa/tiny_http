#include "th_response.h"
#include "th_test.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

static bool
buf_contains(const char* haystack, size_t haystack_len, const char* needle)
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

static bool
plan_contains(const th_response_write_plan* plan, const char* needle)
{
    for (size_t i = 0; i < plan->iovcnt; ++i) {
        if (buf_contains(plan->iov[i].base, plan->iov[i].len, needle))
            return true;
    }
    return false;
}

static bool
plan_start_line_is(const th_response_write_plan* plan, const char* expected)
{
    size_t expected_len = strlen(expected);
    return plan->iov[0].len == expected_len && memcmp(plan->iov[0].base, expected, expected_len) == 0;
}

static bool
plan_has_header_line(const th_response_write_plan* plan, const char* key, const char* value)
{
    char needle[256];
    snprintf(needle, sizeof(needle), "%s: %s\r\n", key, value);
    return plan_contains(plan, needle);
}

static bool
plan_body_is(const th_response_write_plan* plan, const char* expected)
{
    size_t expected_len = strlen(expected);
    if (plan->iovcnt < 3)
        return expected_len == 0;
    return plan->iovcnt >= 3 && plan->iov[2].len == expected_len && memcmp(plan->iov[2].base, expected, expected_len) == 0;
}

typedef struct fake_dir_ops {
    th_dir_ops base;
    int next_fd;
} fake_dir_ops;

static th_err
fake_dir_ops_open(void* self, const char* path, int* fd)
{
    (void)path;
    fake_dir_ops* ops = self;
    *fd = ops->next_fd++;
    return TH_ERR_OK;
}

static void
fake_dir_ops_close(void* self, int fd)
{
    (void)self;
    (void)fd;
}

static void
fake_dir_ops_init(fake_dir_ops* ops)
{
    ops->base.open = fake_dir_ops_open;
    ops->base.close = fake_dir_ops_close;
    ops->next_fd = 3;
}

typedef struct fake_file_ops {
    th_file_ops base;
    int next_fd;
    bool open_fails;
    size_t file_size;
} fake_file_ops;

static th_err
fake_file_ops_openat(void* self, int dirfd, const char* path, int flags, int* fd)
{
    (void)dirfd;
    (void)path;
    (void)flags;
    fake_file_ops* ops = self;
    if (ops->open_fails)
        return TH_ERR_SYSTEM(ENOENT);
    *fd = ops->next_fd++;
    return TH_ERR_OK;
}

static th_err
fake_file_ops_seek(void* self, int fd, int whence, size_t* pos)
{
    (void)fd;
    fake_file_ops* ops = self;
    *pos = (whence == SEEK_END) ? ops->file_size : 0;
    return TH_ERR_OK;
}

static th_err
fake_file_ops_read(void* self, int fd, void* addr, size_t len, size_t offset, size_t* read)
{
    (void)self;
    (void)fd;
    (void)addr;
    (void)offset;
    *read = len;
    return TH_ERR_OK;
}

static th_err
fake_file_ops_write(void* self, int fd, const void* addr, size_t len, size_t offset, size_t* written)
{
    (void)self;
    (void)fd;
    (void)addr;
    (void)offset;
    *written = len;
    return TH_ERR_OK;
}

static th_err
fake_file_ops_stat(void* self, int fd, struct stat* out)
{
    (void)fd;
    fake_file_ops* ops = self;
    *out = (struct stat){0};
    out->st_size = (off_t)ops->file_size;
    return TH_ERR_OK;
}

static void
fake_file_ops_close(void* self, int fd)
{
    (void)self;
    (void)fd;
}

static void
fake_file_ops_init(fake_file_ops* ops)
{
    ops->base.openat = fake_file_ops_openat;
    ops->base.seek = fake_file_ops_seek;
    ops->base.read = fake_file_ops_read;
    ops->base.write = fake_file_ops_write;
    ops->base.stat = fake_file_ops_stat;
    ops->base.close = fake_file_ops_close;
    ops->next_fd = 3;
    ops->open_fails = false;
    ops->file_size = 1234;
}

TH_TEST_BEGIN(response)
{
    fake_dir_ops dir_ops;
    fake_dir_ops_init(&dir_ops);
    th_dir dir;
    th_dir_init(&dir, &dir_ops.base);
    TH_EXPECT(th_dir_open(&dir, TH_STR("/")) == TH_ERR_OK);

    th_dir_mgr dir_mgr;
    th_dir_mgr_init(&dir_mgr, th_default_allocator_get());
    TH_EXPECT(th_dir_mgr_add(&dir_mgr, TH_STR("root"), dir) == TH_ERR_OK);

    fake_file_ops file_ops;
    fake_file_ops_init(&file_ops);
    th_fcache fcache;
    th_fcache_init(&fcache, &file_ops.base, th_default_allocator_get());

    th_response response;
    th_response_init(&response, &dir_mgr, &fcache, th_default_allocator_get());

    TH_TEST_CASE_BEGIN(response_prepare_write_without_content)
    {
        th_response_write_plan plan;
        TH_EXPECT(th_response_prepare_write(&response, &plan) == TH_ERR_OK);

        TH_EXPECT(plan.file == NULL);
        TH_EXPECT(plan_body_is(&plan, ""));
        TH_EXPECT(plan_start_line_is(&plan, "HTTP/1.1 200 OK\r\n"));
        TH_EXPECT(plan_has_header_line(&plan, "Content-Length", "0"));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(response_prepare_write_with_content)
    {
        th_set_body(&response, "Hello, World!");

        th_response_write_plan plan;
        TH_EXPECT(th_response_prepare_write(&response, &plan) == TH_ERR_OK);

        TH_EXPECT(plan.file == NULL);
        TH_EXPECT(plan_body_is(&plan, "Hello, World!"));
        TH_EXPECT(plan_has_header_line(&plan, "Content-Length", "13"));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(response_prepare_write_with_content_and_header)
    {
        th_set_body(&response, "Hello, World!");
        th_add_header(&response, "Connection", "close");
        th_add_header(&response, "Content-Type", "text/plain");

        th_response_write_plan plan;
        TH_EXPECT(th_response_prepare_write(&response, &plan) == TH_ERR_OK);

        TH_EXPECT(plan_has_header_line(&plan, "Connection", "close"));
        TH_EXPECT(plan_has_header_line(&plan, "Content-Type", "text/plain"));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(response_only_headers_skips_body)
    {
        th_set_body(&response, "Hello, World!");
        response.only_headers = true;

        th_response_write_plan plan;
        TH_EXPECT(th_response_prepare_write(&response, &plan) == TH_ERR_OK);

        TH_EXPECT(plan_body_is(&plan, ""));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(response_set_code)
    {
        th_response_set_code(&response, TH_CODE_NOT_FOUND);

        th_response_write_plan plan;
        TH_EXPECT(th_response_prepare_write(&response, &plan) == TH_ERR_OK);

        TH_EXPECT(plan_start_line_is(&plan, "HTTP/1.1 404 Not Found\r\n"));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(response_add_header_rejects_duplicate_known_header)
    {
        // Header ids are looked up case-sensitively against a lowercase
        // gperf table, so the key must be lowercase to be recognized.
        TH_EXPECT(th_add_header(&response, "content-type", "text/plain") == TH_ERR_OK);
        TH_EXPECT(th_add_header(&response, "content-type", "text/html") != TH_ERR_OK);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(response_printf_body_short)
    {
        TH_EXPECT(th_printf_body(&response, "%s is %d", "answer", 42) == TH_ERR_OK);

        th_response_write_plan plan;
        TH_EXPECT(th_response_prepare_write(&response, &plan) == TH_ERR_OK);

        TH_EXPECT(plan_body_is(&plan, "answer is 42"));
        TH_EXPECT(plan_has_header_line(&plan, "Content-Length", "12"));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(response_printf_body_grows_past_stack_buffer)
    {
        // th_response_set_body_va's fast path uses a 512-byte stack
        // buffer; force the slow (resize + reformat) path.
        char long_arg[600];
        memset(long_arg, 'a', sizeof(long_arg) - 1);
        long_arg[sizeof(long_arg) - 1] = '\0';

        TH_EXPECT(th_printf_body(&response, "%s", long_arg) == TH_ERR_OK);

        th_response_write_plan plan;
        TH_EXPECT(th_response_prepare_write(&response, &plan) == TH_ERR_OK);

        TH_EXPECT(plan_body_is(&plan, long_arg));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(response_set_body_from_file)
    {
        file_ops.file_size = 42;
        TH_EXPECT(th_set_body_from_file(&response, "root", "index.html") == TH_ERR_OK);

        th_response_write_plan plan;
        TH_EXPECT(th_response_prepare_write(&response, &plan) == TH_ERR_OK);

        TH_EXPECT(plan.file != NULL);
        TH_EXPECT(plan.offset == 0);
        TH_EXPECT(plan.len == 42);
        TH_EXPECT(plan_has_header_line(&plan, "Content-Type", "text/html"));
        TH_EXPECT(plan_has_header_line(&plan, "Content-Length", "42"));

        file_ops.file_size = 1234;
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(response_set_body_from_file_no_extension)
    {
        TH_EXPECT(th_set_body_from_file(&response, "root", "README") == TH_ERR_OK);

        th_response_write_plan plan;
        TH_EXPECT(th_response_prepare_write(&response, &plan) == TH_ERR_OK);

        TH_EXPECT(plan_has_header_line(&plan, "Content-Type", "application/octet-stream"));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(response_set_body_from_file_unknown_extension)
    {
        TH_EXPECT(th_set_body_from_file(&response, "root", "archive.zzz") == TH_ERR_OK);

        th_response_write_plan plan;
        TH_EXPECT(th_response_prepare_write(&response, &plan) == TH_ERR_OK);

        TH_EXPECT(plan_has_header_line(&plan, "Content-Type", "application/octet-stream"));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(response_set_body_from_file_unknown_root)
    {
        TH_EXPECT(th_set_body_from_file(&response, "does_not_exist", "index.html") != TH_ERR_OK);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(response_set_body_from_file_open_fails)
    {
        file_ops.open_fails = true;
        TH_EXPECT(th_set_body_from_file(&response, "root", "index.html") != TH_ERR_OK);
        file_ops.open_fails = false;
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(response_set_body_from_file_keeps_explicit_content_type)
    {
        TH_EXPECT(th_add_header(&response, "content-type", "application/custom") == TH_ERR_OK);
        TH_EXPECT(th_set_body_from_file(&response, "root", "index.html") == TH_ERR_OK);

        th_response_write_plan plan;
        TH_EXPECT(th_response_prepare_write(&response, &plan) == TH_ERR_OK);

        // th_add_header preserves the caller's casing verbatim.
        TH_EXPECT(plan_has_header_line(&plan, "content-type", "application/custom"));
        TH_EXPECT(!plan_contains(&plan, "text/html"));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(response_add_cookie_minimal)
    {
        TH_EXPECT(th_add_cookie(&response, "session", "abc123", NULL) == TH_ERR_OK);

        th_response_write_plan plan;
        TH_EXPECT(th_response_prepare_write(&response, &plan) == TH_ERR_OK);

        TH_EXPECT(plan_has_header_line(&plan, "Set-Cookie", "session=abc123"));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(response_add_cookie_with_expires)
    {
        th_cookie_attr attr = {0};
        // Mon, 1 Jan 2024 0:0:0 GMT
        attr.expires = (th_date){.year = 124, .month = 0, .day = 1, .weekday = 1, .hour = 0, .minute = 0, .second = 0};

        TH_EXPECT(th_add_cookie(&response, "session", "abc123", &attr) == TH_ERR_OK);

        th_response_write_plan plan;
        TH_EXPECT(th_response_prepare_write(&response, &plan) == TH_ERR_OK);

        TH_EXPECT(plan_has_header_line(&plan, "Set-Cookie", "session=abc123; Expires=Mon, 1 Jan 2024 0:0:0 GMT"));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(response_add_cookie_with_attributes)
    {
        th_cookie_attr attr = {0};
        attr.max_age = th_seconds(3600);
        attr.domain = "example.com";
        attr.path = "/";
        attr.secure = true;
        attr.http_only = true;
        attr.same_site = TH_COOKIE_SAME_SITE_STRICT;

        TH_EXPECT(th_add_cookie(&response, "session", "abc123", &attr) == TH_ERR_OK);

        th_response_write_plan plan;
        TH_EXPECT(th_response_prepare_write(&response, &plan) == TH_ERR_OK);

        TH_EXPECT(plan_has_header_line(&plan, "Set-Cookie",
                                       "session=abc123; Max-Age=3600; Domain=example.com; "
                                       "Path=/; Secure; HttpOnly; SameSite=Strict"));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(response_add_cookie_same_site_none_requires_secure)
    {
        th_cookie_attr attr = {0};
        attr.same_site = TH_COOKIE_SAME_SITE_NONE;
        attr.secure = false;
        TH_EXPECT(th_add_cookie(&response, "session", "abc123", &attr) != TH_ERR_OK);

        attr.secure = true;
        TH_EXPECT(th_add_cookie(&response, "session", "abc123", &attr) == TH_ERR_OK);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(response_reset_clears_body_and_headers)
    {
        th_set_body(&response, "Hello, World!");
        th_add_header(&response, "Connection", "close");
        th_response_set_code(&response, TH_CODE_NOT_FOUND);
        response.only_headers = true;

        th_response_reset(&response);

        th_response_write_plan plan;
        TH_EXPECT(th_response_prepare_write(&response, &plan) == TH_ERR_OK);

        TH_EXPECT(plan_start_line_is(&plan, "HTTP/1.1 200 OK\r\n"));
        TH_EXPECT(!plan_has_header_line(&plan, "Connection", "close"));
        TH_EXPECT(plan_body_is(&plan, ""));
    }
    TH_TEST_CASE_END

    th_response_deinit(&response);
    th_fcache_deinit(&fcache);
    th_dir_mgr_deinit(&dir_mgr);
}
TH_TEST_END
