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
    uint32_t stat_hash;
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

static uint32_t
th_fake_file_ops_stat_hash(void* self, int fd)
{
    (void)fd;
    th_fake_file_ops* ops = self;
    return ops->stat_hash;
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
    ops->base.stat_hash = th_fake_file_ops_stat_hash;
    ops->base.close = th_fake_file_ops_close;
    ops->next_fd = 3;
    ops->open_fails = false;
    ops->stat_hash = 0;
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
    TH_TEST_CASE_BEGIN(fcache_erases_stale_entry_on_hash_mismatch)
    {
        th_fcache cache = {0};
        th_fcache_init(&cache, &file_ops.base, NULL);
        th_fcache_entry* entry1 = NULL;
        TH_EXPECT(th_fcache_get(&cache, &dir, TH_STR("test"), &entry1) == TH_ERR_OK);
        int fd1 = entry1->stream.fd;
        th_fcache_entry_unref(entry1);

        // Simulate the file changing on disk: the cached entry's fd is now
        // stale and must be reopened rather than reused.
        file_ops.stat_hash = 1;
        th_fcache_entry* entry2 = NULL;
        TH_EXPECT(th_fcache_get(&cache, &dir, TH_STR("test"), &entry2) == TH_ERR_OK);
        TH_EXPECT(entry2->stream.fd != fd1);
        TH_EXPECT(cache.num_cached == 1);

        th_fcache_entry_unref(entry2);
        th_fcache_deinit(&cache);
        file_ops.stat_hash = 0;
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(fcache_evicts_oldest_entry_when_full)
    {
        th_fcache cache = {0};
        th_fcache_init(&cache, &file_ops.base, NULL);
        cache.max_cached = 2;
        th_fcache_entry* entry1 = NULL;
        th_fcache_entry* entry2 = NULL;
        th_fcache_entry* entry3 = NULL;
        TH_EXPECT(th_fcache_get(&cache, &dir, TH_STR("a"), &entry1) == TH_ERR_OK);
        TH_EXPECT(th_fcache_get(&cache, &dir, TH_STR("b"), &entry2) == TH_ERR_OK);
        int fd1 = entry1->stream.fd;
        th_fcache_entry_unref(entry1);
        th_fcache_entry_unref(entry2);

        // Cache is now full (max_cached == 2); opening a third file evicts
        // the oldest entry ("a") to make room.
        TH_EXPECT(th_fcache_get(&cache, &dir, TH_STR("c"), &entry3) == TH_ERR_OK);
        TH_EXPECT(cache.num_cached == 2);
        th_fcache_entry_unref(entry3);

        // "a" is no longer cached, so re-requesting it opens a fresh fd
        // rather than reusing fd1.
        th_fcache_entry* entry1_again = NULL;
        TH_EXPECT(th_fcache_get(&cache, &dir, TH_STR("a"), &entry1_again) == TH_ERR_OK);
        TH_EXPECT(entry1_again->stream.fd != fd1);
        th_fcache_entry_unref(entry1_again);

        th_fcache_deinit(&cache);
    }
    TH_TEST_CASE_END

    th_dir_deinit(&dir);
}
TH_TEST_END
