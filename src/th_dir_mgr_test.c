#include "th_dir_mgr.h"
#include "th_test.h"

TH_TEST_BEGIN(dir_mgr)
{
    TH_TEST_CASE_BEGIN(dir_mgr_init)
    {
        th_dir_mgr mgr = {0};
        th_dir_mgr_init(&mgr, NULL);
        th_dir_mgr_deinit(&mgr);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(dir_mgr_add)
    {
        th_dir_mgr mgr = {0};
        th_dir_mgr_init(&mgr, NULL);
        TH_EXPECT(th_dir_mgr_add(&mgr, TH_STRING("test"), TH_STRING("/")) == TH_ERR_OK);
        TH_EXPECT(th_dir_mgr_get(&mgr, TH_STRING("test")) != NULL);
        th_dir_mgr_deinit(&mgr);
    }
    TH_TEST_CASE_END
}
TH_TEST_END
