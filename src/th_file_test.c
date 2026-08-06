#include "th_config.h"
#include "th_file.h"
#include "th_test.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

typedef struct th_fake_file_ops {
    th_file_ops base;
    int next_fd;
    int last_dirfd;
    char last_path[64];
    int last_flags;
    bool open_fails;
    size_t file_size;
    size_t seek_calls;
    bool seek_fails;
    size_t read_len;
    bool read_fails;
    size_t write_len;
    bool write_fails;
    struct stat stat_value;
    bool stat_fails;
    int closed_fd;
    size_t close_calls;
} th_fake_file_ops;

static th_err
th_fake_file_ops_openat(void* self, int dirfd, const char* path, int flags, int* fd)
{
    th_fake_file_ops* ops = self;
    ops->last_dirfd = dirfd;
    strncpy(ops->last_path, path, sizeof(ops->last_path) - 1);
    ops->last_flags = flags;
    if (ops->open_fails)
        return TH_ERR_SYSTEM(ENOENT);
    *fd = ops->next_fd++;
    return TH_ERR_OK;
}

static th_err
th_fake_file_ops_seek(void* self, int fd, int whence, size_t* pos)
{
    (void)fd;
    th_fake_file_ops* ops = self;
    ops->seek_calls++;
    if (ops->seek_fails)
        return TH_ERR_SYSTEM(EIO);
    *pos = whence == SEEK_END ? ops->file_size : 0;
    return TH_ERR_OK;
}

static th_err
th_fake_file_ops_read(void* self, int fd, void* addr, size_t len, size_t offset, size_t* read)
{
    (void)fd;
    (void)addr;
    (void)len;
    (void)offset;
    th_fake_file_ops* ops = self;
    if (ops->read_fails)
        return TH_ERR_SYSTEM(EIO);
    *read = ops->read_len;
    return TH_ERR_OK;
}

static th_err
th_fake_file_ops_write(void* self, int fd, const void* addr, size_t len, size_t offset, size_t* written)
{
    (void)fd;
    (void)addr;
    (void)len;
    (void)offset;
    th_fake_file_ops* ops = self;
    if (ops->write_fails)
        return TH_ERR_SYSTEM(EIO);
    *written = ops->write_len;
    return TH_ERR_OK;
}

static th_err
th_fake_file_ops_stat(void* self, int fd, struct stat* out)
{
    (void)fd;
    th_fake_file_ops* ops = self;
    if (ops->stat_fails)
        return TH_ERR_SYSTEM(EIO);
    *out = ops->stat_value;
    return TH_ERR_OK;
}

static void
th_fake_file_ops_close(void* self, int fd)
{
    th_fake_file_ops* ops = self;
    ops->closed_fd = fd;
    ops->close_calls++;
}

static void
th_fake_file_ops_init(th_fake_file_ops* ops)
{
    memset(ops, 0, sizeof(*ops));
    ops->base.openat = th_fake_file_ops_openat;
    ops->base.seek = th_fake_file_ops_seek;
    ops->base.read = th_fake_file_ops_read;
    ops->base.write = th_fake_file_ops_write;
    ops->base.stat = th_fake_file_ops_stat;
    ops->base.close = th_fake_file_ops_close;
    ops->next_fd = 3;
    ops->closed_fd = -1;
}

TH_TEST_BEGIN(file)
{
    th_fake_file_ops ops;
    th_dir dir = {0};
    dir.fd = 42;

    TH_TEST_CASE_BEGIN(file_openat_success_sets_fd_and_size)
    {
        th_fake_file_ops_init(&ops);
        ops.file_size = 1234;
        th_filepath path;
        TH_EXPECT(th_filepath_init(&path, TH_STR("foo.txt")) == TH_ERR_OK);
        th_file file;
        th_file_init(&file, &ops.base);
        th_open_opt opt = {.read = true};
        TH_EXPECT(th_file_openat(&file, &dir, &path, opt) == TH_ERR_OK);
        TH_EXPECT(file.fd == 3);
        TH_EXPECT(file.size == 1234);
        TH_EXPECT(ops.last_dirfd == 42);
        TH_EXPECT(strcmp(ops.last_path, "foo.txt") == 0);
        TH_EXPECT(ops.seek_calls == 2);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(file_openat_propagates_openat_failure)
    {
        th_fake_file_ops_init(&ops);
        ops.open_fails = true;
        th_filepath path;
        TH_EXPECT(th_filepath_init(&path, TH_STR("foo.txt")) == TH_ERR_OK);
        th_file file;
        th_file_init(&file, &ops.base);
        th_open_opt opt = {.read = true};
        TH_EXPECT(th_file_openat(&file, &dir, &path, opt) == TH_ERR_SYSTEM(ENOENT));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(file_openat_closes_fd_on_seek_failure)
    {
        th_fake_file_ops_init(&ops);
        ops.seek_fails = true;
        th_filepath path;
        TH_EXPECT(th_filepath_init(&path, TH_STR("foo.txt")) == TH_ERR_OK);
        th_file file;
        th_file_init(&file, &ops.base);
        th_open_opt opt = {.read = true};
        TH_EXPECT(th_file_openat(&file, &dir, &path, opt) == TH_ERR_SYSTEM(EIO));
        TH_EXPECT(ops.close_calls == 1);
        TH_EXPECT(ops.closed_fd == 3);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(file_openat_translates_read_write_flags)
    {
        th_fake_file_ops_init(&ops);
        th_filepath path;
        TH_EXPECT(th_filepath_init(&path, TH_STR("foo.txt")) == TH_ERR_OK);
        th_file file;
        th_file_init(&file, &ops.base);
        th_open_opt opt = {.create = true, .write = true, .truncate = true};
        TH_EXPECT(th_file_openat(&file, &dir, &path, opt) == TH_ERR_OK);
        TH_EXPECT((ops.last_flags & O_WRONLY) != 0);
        TH_EXPECT((ops.last_flags & O_CREAT) != 0);
        TH_EXPECT((ops.last_flags & O_TRUNC) != 0);
        TH_EXPECT((ops.last_flags & O_NOFOLLOW) != 0);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(file_read_delegates_to_ops)
    {
        th_fake_file_ops_init(&ops);
        ops.read_len = 42;
        th_file file;
        th_file_init(&file, &ops.base);
        file.fd = 7;
        char buffer[8];
        size_t read = 0;
        TH_EXPECT(th_file_read(&file, buffer, sizeof(buffer), 0, &read) == TH_ERR_OK);
        TH_EXPECT(read == 42);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(file_read_propagates_failure)
    {
        th_fake_file_ops_init(&ops);
        ops.read_fails = true;
        th_file file;
        th_file_init(&file, &ops.base);
        file.fd = 7;
        char buffer[8];
        size_t read = 0;
        TH_EXPECT(th_file_read(&file, buffer, sizeof(buffer), 0, &read) == TH_ERR_SYSTEM(EIO));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(file_write_delegates_to_ops)
    {
        th_fake_file_ops_init(&ops);
        ops.write_len = 42;
        th_file file;
        th_file_init(&file, &ops.base);
        file.fd = 7;
        const char buffer[8] = {0};
        size_t written = 0;
        TH_EXPECT(th_file_write(&file, buffer, sizeof(buffer), 0, &written) == TH_ERR_OK);
        TH_EXPECT(written == 42);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(file_write_propagates_failure)
    {
        th_fake_file_ops_init(&ops);
        ops.write_fails = true;
        th_file file;
        th_file_init(&file, &ops.base);
        file.fd = 7;
        const char buffer[8] = {0};
        size_t written = 0;
        TH_EXPECT(th_file_write(&file, buffer, sizeof(buffer), 0, &written) == TH_ERR_SYSTEM(EIO));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(file_stat_hash_differs_on_different_stat)
    {
        th_fake_file_ops_init(&ops);
        th_file file;
        th_file_init(&file, &ops.base);
        file.fd = 7;
        ops.stat_value = (struct stat){.st_ino = 1, .st_size = 100};
        uint32_t hash1 = th_file_stat_hash(&file);
        ops.stat_value = (struct stat){.st_ino = 2, .st_size = 100};
        uint32_t hash2 = th_file_stat_hash(&file);
        TH_EXPECT(hash1 != hash2);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(file_stat_hash_same_on_same_stat)
    {
        th_fake_file_ops_init(&ops);
        th_file file;
        th_file_init(&file, &ops.base);
        file.fd = 7;
        ops.stat_value = (struct stat){.st_ino = 1, .st_size = 100};
#if defined(TH_CONFIG_OS_OSX)
        ops.stat_value.st_mtimespec.tv_sec = 5;
#else
        ops.stat_value.st_mtime = 5;
#endif
        uint32_t hash1 = th_file_stat_hash(&file);
        uint32_t hash2 = th_file_stat_hash(&file);
        TH_EXPECT(hash1 == hash2);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(file_close_calls_ops_close_once)
    {
        th_fake_file_ops_init(&ops);
        th_file file;
        th_file_init(&file, &ops.base);
        file.fd = 9;
        th_file_close(&file);
        TH_EXPECT(ops.close_calls == 1);
        TH_EXPECT(ops.closed_fd == 9);
        TH_EXPECT(file.fd == -1);
        // Closing again must be a no-op - fd is already -1.
        th_file_close(&file);
        TH_EXPECT(ops.close_calls == 1);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(file_close_on_never_opened_file_is_noop)
    {
        th_fake_file_ops_init(&ops);
        th_file file;
        th_file_init(&file, &ops.base);
        th_file_close(&file);
        TH_EXPECT(ops.close_calls == 0);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(file_deinit_closes_open_fd)
    {
        th_fake_file_ops_init(&ops);
        th_file file;
        th_file_init(&file, &ops.base);
        file.fd = 11;
        th_file_deinit(&file);
        TH_EXPECT(ops.close_calls == 1);
        TH_EXPECT(ops.closed_fd == 11);
    }
    TH_TEST_CASE_END
}
TH_TEST_END
