#include "th_dir_mgr.h"
#include "th_test.h"

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

TH_TEST_BEGIN(dir_mgr)
{
    th_fake_dir_ops ops;
    th_fake_dir_ops_init(&ops);
    th_dir_mgr mgr = {0};
    th_dir_mgr_init(&mgr, NULL);

    TH_TEST_CASE_BEGIN(dir_mgr_init)
    {
        th_dir_mgr_deinit(&mgr);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(dir_mgr_add)
    {
        th_dir dir;
        th_dir_init(&dir, &ops.base);
        TH_EXPECT(th_dir_open(&dir, TH_STR("/")) == TH_ERR_OK);

        TH_EXPECT(th_dir_mgr_add(&mgr, TH_STR("test"), dir) == TH_ERR_OK);
        TH_EXPECT(th_dir_mgr_get(&mgr, TH_STR("test")) != NULL);
        th_dir_mgr_deinit(&mgr);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(dir_mgr_add_survives_alloc_failure_at_every_step)
    {
        int n = 0;
        th_err err;
        do {
            th_dir dir;
            th_dir_init(&dir, &ops.base);
            TH_EXPECT(th_dir_open(&dir, TH_STR("/")) == TH_ERR_OK);

            th_test_allocator_fail_after(n++);
            err = th_dir_mgr_add(&mgr, TH_STR("justsomelongdirnametotriggeralloc"), dir);
            TH_EXPECT(err == TH_ERR_OK || err == TH_ERR_BAD_ALLOC);
            TH_EXPECT(n < 1000);
        } while (err != TH_ERR_OK);

        TH_EXPECT(th_dir_mgr_get(&mgr, TH_STR("justsomelongdirnametotriggeralloc")) != NULL);
        th_dir_mgr_deinit(&mgr);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(dir_mgr_add_duplicate_label)
    {
        th_dir dir1;
        th_dir_init(&dir1, &ops.base);
        TH_EXPECT(th_dir_open(&dir1, TH_STR("/")) == TH_ERR_OK);
        TH_EXPECT(th_dir_mgr_add(&mgr, TH_STR("test"), dir1) == TH_ERR_OK);

        th_dir dir2;
        th_dir_init(&dir2, &ops.base);
        TH_EXPECT(th_dir_open(&dir2, TH_STR("/")) == TH_ERR_OK);
        TH_EXPECT(th_dir_mgr_add(&mgr, TH_STR("test"), dir2) == TH_ERR_INVALID_ARG);

        th_dir_mgr_deinit(&mgr);
    }
    TH_TEST_CASE_END
}
TH_TEST_END
