#include "th_string.h"
#include "th_test.h"
#include "th_url_decode.h"

TH_TEST_BEGIN(url_decode)
{
    TH_TEST_CASE_BEGIN(url_decode_empty)
    {
        th_string output;
        th_string_init(&output, th_default_allocator_get());
        TH_EXPECT(th_url_decode_string(TH_STR(""), &output, TH_URL_DECODE_TYPE_PATH) == TH_ERR_OK);
        TH_EXPECT(th_string_len(&output) == 0);
        th_string_deinit(&output);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(url_decode_nothing)
    {
        th_string output;
        th_string_init(&output, th_default_allocator_get());
        TH_EXPECT(th_url_decode_string(TH_STR("hello"), &output, TH_URL_DECODE_TYPE_PATH) == TH_ERR_OK);
        TH_EXPECT(TH_STR_EQ(th_string_view(&output), "hello"));
        th_string_deinit(&output);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(url_decode_space)
    {
        th_string output;
        th_string_init(&output, th_default_allocator_get());
        TH_EXPECT(th_url_decode_string(TH_STR("hello%20world"), &output, TH_URL_DECODE_TYPE_PATH) == TH_ERR_OK);
        TH_EXPECT(TH_STR_EQ(th_string_view(&output), "hello world"));
        th_string_deinit(&output);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(url_decode_bad_request)
    {
        th_string output;
        th_string_init(&output, th_default_allocator_get());
        TH_EXPECT(
            th_url_decode_string(TH_STR("hello%2"), &output, TH_URL_DECODE_TYPE_PATH)
            == TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
        th_string_deinit(&output);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(url_decode_plus_as_space_in_query)
    {
        th_string output;
        th_string_init(&output, th_default_allocator_get());
        TH_EXPECT(th_url_decode_string(TH_STR("hello+world"), &output, TH_URL_DECODE_TYPE_QUERY) == TH_ERR_OK);
        TH_EXPECT(TH_STR_EQ(th_string_view(&output), "hello world"));
        th_string_deinit(&output);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(url_decode_plus_kept_literal_in_path)
    {
        th_string output;
        th_string_init(&output, th_default_allocator_get());
        TH_EXPECT(th_url_decode_string(TH_STR("hello+world"), &output, TH_URL_DECODE_TYPE_PATH) == TH_ERR_OK);
        TH_EXPECT(TH_STR_EQ(th_string_view(&output), "hello+world"));
        th_string_deinit(&output);
    }
    TH_TEST_CASE_END
}
TH_TEST_END
