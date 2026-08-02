#include "th_fcache.h"
#include "th_mock_syscall.h"
#include "th_test.h"

#include <errno.h>

static int th_mock_open_bad(void)
{
    return -TH_ENOENT;
}

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

TH_TEST_BEGIN(fcache)
{
    th_fake_dir_ops dir_ops;
    th_fake_dir_ops_init(&dir_ops);
    th_dir_mgr dir_mgr;
    th_dir_mgr_init(&dir_mgr, NULL);
    th_dir dir;
    th_dir_init(&dir, &dir_ops.base, NULL);
    TH_EXPECT(th_dir_open(&dir, TH_STR("/")) == TH_ERR_OK);
    TH_EXPECT(th_dir_mgr_add(&dir_mgr, TH_STR("/"), dir) == TH_ERR_OK);

    TH_TEST_CASE_BEGIN(fcache_init)
    {
        th_fcache cache = {0};
        th_fcache_init(&cache, &dir_mgr, NULL);
        th_fcache_deinit(&cache);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(fcache_open)
    {
        th_fcache cache = {0};
        th_fcache_init(&cache, &dir_mgr, NULL);
        th_fcache_entry* entry1 = NULL;
        th_fcache_entry* entry2 = NULL;
        th_fcache_entry* entry3 = NULL;
        TH_EXPECT(th_fcache_get(&cache, TH_STR("/"), TH_STR("test"), &entry1) == TH_ERR_OK);
        TH_EXPECT(th_fcache_get(&cache, TH_STR("/"), TH_STR("test"), &entry2) == TH_ERR_OK);
        TH_EXPECT(th_fcache_get(&cache, TH_STR("/"), TH_STR("test"), &entry3) == TH_ERR_OK);
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
        th_fcache_init(&cache, &dir_mgr, NULL);
        th_mock_syscall_get()->open = th_mock_open_bad;
        th_fcache_entry* entry = NULL;
        TH_EXPECT(th_fcache_get(&cache, TH_STR("/"), TH_STR("test"), &entry) != TH_ERR_OK);
        th_fcache_deinit(&cache);
        th_mock_syscall_reset();
    }
    TH_TEST_CASE_END

    th_dir_mgr_deinit(&dir_mgr);
}
TH_TEST_END
