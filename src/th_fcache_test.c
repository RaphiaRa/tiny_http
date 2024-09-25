#include "th_fcache.h"
#include "th_mock_syscall.h"
#include "th_test.h"

#include <errno.h>

static int th_mock_open_bad(void)
{
    return -TH_ENOENT;
}

TH_TEST_BEGIN(fcache)
{
    TH_TEST_CASE_BEGIN(fcache_init)
    {
        th_fcache cache = {0};
        th_fcache_init(&cache, NULL);
        th_fcache_deinit(&cache);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(fcache_open)
    {
        th_fcache cache = {0};
        th_fcache_init(&cache, NULL);
        th_fcache_entry* entry1 = NULL;
        th_fcache_entry* entry2 = NULL;
        th_fcache_entry* entry3 = NULL;
        TH_EXPECT(th_fcache_add_root(&cache, TH_STRING("/"), TH_STRING("/")) == TH_ERR_OK);
        TH_EXPECT(th_fcache_get(&cache, TH_STRING("/"), TH_STRING("test"), &entry1) == TH_ERR_OK);
        TH_EXPECT(th_fcache_get(&cache, TH_STRING("/"), TH_STRING("test"), &entry2) == TH_ERR_OK);
        TH_EXPECT(th_fcache_get(&cache, TH_STRING("/"), TH_STRING("test"), &entry3) == TH_ERR_OK);
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
        th_fcache_init(&cache, NULL);
        TH_EXPECT(th_fcache_add_root(&cache, TH_STRING("/"), TH_STRING("/")) == TH_ERR_OK);
        th_mock_syscall_get()->open = th_mock_open_bad;
        th_fcache_entry* entry = NULL;
        TH_EXPECT(th_fcache_get(&cache, TH_STRING("/"), TH_STRING("test"), &entry) != TH_ERR_OK);
        th_fcache_deinit(&cache);
        th_mock_syscall_reset();
    }
    TH_TEST_CASE_END
}
TH_TEST_END
