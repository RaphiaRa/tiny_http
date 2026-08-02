#include "th_dir.h"
#include "th_system_error.h"
#include "th_test.h"

typedef struct th_fake_dir_ops {
    th_dir_ops base;
    th_err open_err;
    int next_fd;
    int closed_fd;
    int close_calls;
} th_fake_dir_ops;

static th_err
th_fake_dir_ops_open(void* self, const char* path, int* fd)
{
    (void)path;
    th_fake_dir_ops* ops = self;
    if (ops->open_err != TH_ERR_OK)
        return ops->open_err;
    *fd = ops->next_fd++;
    return TH_ERR_OK;
}

static void
th_fake_dir_ops_close(void* self, int fd)
{
    th_fake_dir_ops* ops = self;
    ops->closed_fd = fd;
    ++ops->close_calls;
}

static void
th_fake_dir_ops_init(th_fake_dir_ops* ops)
{
    ops->base.open = th_fake_dir_ops_open;
    ops->base.close = th_fake_dir_ops_close;
    ops->open_err = TH_ERR_OK;
    ops->next_fd = 3;
    ops->closed_fd = -1;
    ops->close_calls = 0;
}

TH_TEST_BEGIN(dir)
{
    th_fake_dir_ops ops;
    th_fake_dir_ops_init(&ops);
    th_dir dir;
    th_dir_init(&dir, &ops.base, NULL);

    TH_TEST_CASE_BEGIN(dir_open_uses_injected_ops)
    {
        TH_EXPECT(th_dir_open(&dir, TH_STR("/")) == TH_ERR_OK);
        TH_EXPECT(th_str_eq(th_dir_get_path(&dir), TH_STR("/")));

        th_dir_deinit(&dir);
        TH_EXPECT(ops.close_calls == 1);
        TH_EXPECT(ops.closed_fd == 3);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(dir_open_propagates_ops_error)
    {
        ops.open_err = TH_ERR_SYSTEM(TH_ENOENT);

        TH_EXPECT(th_dir_open(&dir, TH_STR("/")) == TH_ERR_SYSTEM(TH_ENOENT));

        th_dir_deinit(&dir);
        TH_EXPECT(ops.close_calls == 0);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(dir_deinit_without_open_does_not_close)
    {
        th_dir_deinit(&dir);
        TH_EXPECT(ops.close_calls == 0);
    }
    TH_TEST_CASE_END
}
TH_TEST_END
