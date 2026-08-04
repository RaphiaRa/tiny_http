#include "th_bench.h"
#include "th_url_decode.h"

TH_BENCH_BEGIN(url_decode)
{
    th_string output;
    th_string_init(&output, NULL);
    th_string_resize(&output, 4096, '\0');
    th_string_clear(&output);

    TH_BENCH_CASE_BEGIN(decode_short_no_encoding, 1000000)
    {
        th_str input = TH_STR("articles");

        TH_BENCH_RUN_BEGIN
        {
            th_string_clear(&output);
            th_url_decode_string(input, &output, TH_URL_DECODE_TYPE_PATH);
        }
        TH_BENCH_RUN_END
    }
    TH_BENCH_CASE_END

    TH_BENCH_CASE_BEGIN(decode_short_mixed, 500000)
    {
        th_str input = TH_STR("hello%20world%21");

        TH_BENCH_RUN_BEGIN
        {
            th_string_clear(&output);
            th_url_decode_string(input, &output, TH_URL_DECODE_TYPE_PATH);
        }
        TH_BENCH_RUN_END
    }
    TH_BENCH_CASE_END

    TH_BENCH_CASE_BEGIN(decode_short_all_encoded, 500000)
    {
        th_str input = TH_STR("%68%65%6C%6C%6F%20%77%6F%72%6C%64");

        TH_BENCH_RUN_BEGIN
        {
            th_string_clear(&output);
            th_url_decode_string(input, &output, TH_URL_DECODE_TYPE_PATH);
        }
        TH_BENCH_RUN_END
    }
    TH_BENCH_CASE_END

    TH_BENCH_CASE_BEGIN(decode_query_plus_as_space, 500000)
    {
        th_str input = TH_STR("key1=value+one&key2=value+two");

        TH_BENCH_RUN_BEGIN
        {
            th_string_clear(&output);
            th_url_decode_string(input, &output, TH_URL_DECODE_TYPE_QUERY);
        }
        TH_BENCH_RUN_END
    }
    TH_BENCH_CASE_END

    TH_BENCH_CASE_BEGIN(decode_4kb_sparse_encoding, 10000)
    {
        static char buf[4096];
        for (size_t i = 0; i < sizeof(buf); i++)
            buf[i] = 'a' + (char)(i % 26);
        for (size_t i = 0; i + 2 < sizeof(buf); i += 128) {
            buf[i] = '%';
            buf[i + 1] = '2';
            buf[i + 2] = '0';
        }
        th_str input = th_str_make(buf, sizeof(buf));

        TH_BENCH_RUN_BEGIN
        {
            th_string_clear(&output);
            th_url_decode_string(input, &output, TH_URL_DECODE_TYPE_PATH);
        }
        TH_BENCH_RUN_END
    }
    TH_BENCH_CASE_END

    th_string_deinit(&output);
}
TH_BENCH_END
