#include "th_fcache.h"
#include "th_test.h"

#include <errno.h>

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
th_fake_file_ops_read(void* self, int fd, void* addr, size_t len, size_t offset, size_t* read)
{
    (void)self;
    (void)fd;
    (void)addr;
    (void)offset;
    *read = len;
    return TH_ERR_OK;
}

static th_err
th_fake_file_ops_write(void* self, int fd, const void* addr, size_t len, size_t offset, size_t* written)
{
    (void)self;
    (void)fd;
    (void)addr;
    (void)offset;
    *written = len;
    return TH_ERR_OK;
}

static th_err
th_fake_file_ops_mmap(void* self, int fd, size_t offset, size_t len, void** addr, size_t* mapped_offset, size_t* mapped_len)
{
    (void)self;
    (void)fd;
    (void)offset;
    (void)len;
    *addr = NULL;
    *mapped_offset = 0;
    *mapped_len = 0;
    return TH_ERR_NOSUPPORT;
}

static void
th_fake_file_ops_munmap(void* self, void* addr, size_t len)
{
    (void)self;
    (void)addr;
    (void)len;
}

static uint32_t
th_fake_file_ops_stat_hash(void* self, int fd)
{
    (void)self;
    (void)fd;
    return 0;
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
    ops->base.read = th_fake_file_ops_read;
    ops->base.write = th_fake_file_ops_write;
    ops->base.mmap = th_fake_file_ops_mmap;
    ops->base.munmap = th_fake_file_ops_munmap;
    ops->base.stat_hash = th_fake_file_ops_stat_hash;
    ops->base.close = th_fake_file_ops_close;
    ops->next_fd = 3;
    ops->open_fails = false;
}

TH_TEST_BEGIN(fcache)
{
    th_fake_dir_ops dir_ops;
    th_fake_dir_ops_init(&dir_ops);
    th_dir dir;
    th_dir_init(&dir, &dir_ops.base);
    TH_EXPECT(th_dir_open(&dir, TH_STR("/")) == TH_ERR_OK);
    th_fake_file_ops file_ops;
    th_fake_file_ops_init(&file_ops);

    TH_TEST_CASE_BEGIN(fcache_init)
    {
        th_fcache cache = {0};
        th_fcache_init(&cache, &file_ops.base, NULL);
        th_fcache_deinit(&cache);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(fcache_open)
    {
        th_fcache cache = {0};
        th_fcache_init(&cache, &file_ops.base, NULL);
        th_fcache_entry* entry1 = NULL;
        th_fcache_entry* entry2 = NULL;
        th_fcache_entry* entry3 = NULL;
        TH_EXPECT(th_fcache_get(&cache, &dir, TH_STR("test"), &entry1) == TH_ERR_OK);
        TH_EXPECT(th_fcache_get(&cache, &dir, TH_STR("test"), &entry2) == TH_ERR_OK);
        TH_EXPECT(th_fcache_get(&cache, &dir, TH_STR("test"), &entry3) == TH_ERR_OK);
        TH_EXPECT(entry1->stream.fd == entry2->stream.fd);
        TH_EXPECT(entry2->stream.fd == entry3->stream.fd);
        th_fcache_entry_unref(entry1);
        th_fcache_entry_unref(entry2);
        th_fcache_entry_unref(entry3);
        th_fcache_deinit(&cache);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(fcache_open_bad)
    {
        th_fcache cache = {0};
        th_fcache_init(&cache, &file_ops.base, NULL);
        file_ops.open_fails = true;
        th_fcache_entry* entry = NULL;
        TH_EXPECT(th_fcache_get(&cache, &dir, TH_STR("test"), &entry) != TH_ERR_OK);
        th_fcache_deinit(&cache);
        file_ops.open_fails = false;
    }
    TH_TEST_CASE_END

    th_dir_deinit(&dir);
}
TH_TEST_END
