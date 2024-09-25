#include "th_test.h"
#include "th_url_decode.h"

TH_TEST_BEGIN(url_decode)
{
    TH_TEST_CASE_BEGIN(url_decode_empty)
    {
        char str[] = "";
        size_t len = sizeof(str) - 1;
        TH_EXPECT(th_url_decode_inplace(str, &len, TH_URL_DECODE_TYPE_PATH) == TH_ERR_OK);
        TH_EXPECT(len == 0);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(url_decode_nothing)
    {
        char str[] = "hello";
        size_t len = sizeof(str) - 1;
        TH_EXPECT(th_url_decode_inplace(str, &len, TH_URL_DECODE_TYPE_PATH) == TH_ERR_OK);
        TH_EXPECT(len == 5);
        TH_EXPECT(strcmp(str, "hello") == 0);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(url_decode_space)
    {
        char str[] = "hello%20world";
        size_t len = sizeof(str) - 1;
        TH_EXPECT(th_url_decode_inplace(str, &len, TH_URL_DECODE_TYPE_PATH) == TH_ERR_OK);
        TH_EXPECT(len == 11);
        TH_EXPECT(strcmp(str, "hello world") == 0);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(url_decode_bad_request)
    {
        char str[] = "hello%2";
        size_t len = sizeof(str) - 1;
        TH_EXPECT(th_url_decode_inplace(str, &len, TH_URL_DECODE_TYPE_PATH) == TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
    }
    TH_TEST_CASE_END
}
TH_TEST_END
