#include "th_test.h"
#include "th_upload.h"

#include <errno.h>
#include <string.h>

typedef struct th_fake_dir_ops {
    th_dir_ops base;
    int next_fd;
} th_fake_dir_ops;

static th_err
th_fake_dir_ops_open(void* self, const char* path, int* fd)
{
    (void)path;
    th_fake_dir_ops* ops = self;
    *fd = ops->next_fd++;
    return TH_ERR_OK;
}

static void
th_fake_dir_ops_close(void* self, int fd)
{
    (void)self;
    (void)fd;
}

static void
th_fake_dir_ops_init(th_fake_dir_ops* ops)
{
    ops->base.open = th_fake_dir_ops_open;
    ops->base.close = th_fake_dir_ops_close;
    ops->next_fd = 3;
}

typedef struct th_fake_file_ops {
    th_file_ops base;
    int next_fd;
    bool open_fails;
    bool write_fails;
    char written[1024];
    size_t written_len;
} th_fake_file_ops;

static th_err
th_fake_file_ops_openat(void* self, th_dir* dir, th_str path, th_open_opt opt, int* fd, size_t* size)
{
    (void)dir;
    (void)path;
    (void)opt;
    th_fake_file_ops* ops = self;
    if (ops->open_fails)
        return TH_ERR_SYSTEM(ENOENT);
    *fd = ops->next_fd++;
    *size = 0;
    return TH_ERR_OK;
}

static th_err
th_fake_file_ops_write(void* self, int fd, const void* addr, size_t len, size_t offset, size_t* written)
{
    (void)fd;
    th_fake_file_ops* ops = self;
    if (ops->write_fails)
        return TH_ERR_SYSTEM(EIO);
    memcpy(ops->written + offset, addr, len);
    if (offset + len > ops->written_len)
        ops->written_len = offset + len;
    *written = len;
    return TH_ERR_OK;
}

static void
th_fake_file_ops_close(void* self, int fd)
{
    (void)self;
    (void)fd;
}

static void
th_fake_file_ops_init(th_fake_file_ops* ops)
{
    ops->base.openat = th_fake_file_ops_openat;
    ops->base.read = NULL;
    ops->base.write = th_fake_file_ops_write;
    ops->base.stat_hash = NULL;
    ops->base.close = th_fake_file_ops_close;
    ops->next_fd = 3;
    ops->open_fails = false;
    ops->write_fails = false;
    ops->written_len = 0;
}

TH_TEST_BEGIN(upload)
{
    th_fake_dir_ops dir_ops;
    th_fake_dir_ops_init(&dir_ops);
    th_dir dir;
    th_dir_init(&dir, &dir_ops.base);
    TH_EXPECT(th_dir_open(&dir, TH_STR("/")) == TH_ERR_OK);
    th_dir_mgr dir_mgr;
    th_dir_mgr_init(&dir_mgr, th_default_allocator_get());
    TH_EXPECT(th_dir_mgr_add(&dir_mgr, TH_STR("uploads"), dir) == TH_ERR_OK);
    th_fake_file_ops file_ops;
    th_fake_file_ops_init(&file_ops);

    TH_TEST_CASE_BEGIN(upload_init_sets_data)
    {
        th_upload upload;
        th_upload_init(&upload, TH_STR("hello"), &dir_mgr, &file_ops.base, th_default_allocator_get());

        th_buffer data = th_upload_get_data(&upload);
        TH_EXPECT(data.len == 5);
        TH_EXPECT(memcmp(data.ptr, "hello", 5) == 0);

        th_upload_deinit(&upload);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(upload_set_fields_reflected_in_info)
    {
        th_upload upload;
        th_upload_init(&upload, TH_STR("hello"), &dir_mgr, &file_ops.base, th_default_allocator_get());
        TH_EXPECT(th_upload_set_name(&upload, TH_STR("file")) == TH_ERR_OK);
        TH_EXPECT(th_upload_set_filename(&upload, TH_STR("test.txt")) == TH_ERR_OK);
        TH_EXPECT(th_upload_set_content_type(&upload, TH_STR("text/plain")) == TH_ERR_OK);

        th_upload_info info = th_upload_get_info(&upload);
        TH_EXPECT(strcmp(info.name, "file") == 0);
        TH_EXPECT(strcmp(info.filename, "test.txt") == 0);
        TH_EXPECT(strcmp(info.content_type, "text/plain") == 0);
        TH_EXPECT(info.size == 5);

        th_upload_deinit(&upload);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(upload_save_writes_data_via_file_ops)
    {
        th_upload upload;
        th_upload_init(&upload, TH_STR("hello, world"), &dir_mgr, &file_ops.base, th_default_allocator_get());

        TH_EXPECT(th_upload_save(&upload, "uploads", "test.txt") == TH_ERR_OK);
        TH_EXPECT(file_ops.written_len == 12);
        TH_EXPECT(memcmp(file_ops.written, "hello, world", 12) == 0);

        th_upload_deinit(&upload);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(upload_save_unknown_dir_label_fails)
    {
        th_upload upload;
        th_upload_init(&upload, TH_STR("hello"), &dir_mgr, &file_ops.base, th_default_allocator_get());

        TH_EXPECT(th_upload_save(&upload, "no_such_dir", "test.txt") == TH_ERR_HTTP(TH_CODE_NOT_FOUND));

        th_upload_deinit(&upload);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(upload_save_propagates_open_error)
    {
        th_upload upload;
        th_upload_init(&upload, TH_STR("hello"), &dir_mgr, &file_ops.base, th_default_allocator_get());
        file_ops.open_fails = true;

        TH_EXPECT(th_upload_save(&upload, "uploads", "test.txt") == TH_ERR_SYSTEM(ENOENT));

        file_ops.open_fails = false;
        th_upload_deinit(&upload);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(upload_save_propagates_write_error)
    {
        th_upload upload;
        th_upload_init(&upload, TH_STR("hello"), &dir_mgr, &file_ops.base, th_default_allocator_get());
        file_ops.write_fails = true;

        TH_EXPECT(th_upload_save(&upload, "uploads", "test.txt") == TH_ERR_SYSTEM(EIO));

        file_ops.write_fails = false;
        th_upload_deinit(&upload);
    }
    TH_TEST_CASE_END

    th_dir_mgr_deinit(&dir_mgr);
}
TH_TEST_END
