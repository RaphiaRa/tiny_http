#include "th_bench.h"
#include "th_cookie_parser.h"

TH_BENCH_BEGIN(cookie_parser)
{
    TH_BENCH_CASE_BEGIN(parse_single_cookie, 100000)
    {
        th_str header = TH_STR("session_id=abc123def456");

        TH_BENCH_RUN_BEGIN
        {
            th_cookie_parser parser;
            th_cookie_parser_init(&parser, header);
            while (!th_cookie_parser_done(&parser)) {
                th_str key, value;
                th_cookie_parser_next(&parser, &key, &value);
            }
        }
        TH_BENCH_RUN_END
    }
    TH_BENCH_CASE_END

    TH_BENCH_CASE_BEGIN(parse_ten_cookies, 10000)
    {
        th_str header = TH_STR("a=1; b=2; c=3; d=4; e=5; f=6; g=7; h=8; i=9; j=10");

        TH_BENCH_RUN_BEGIN
        {
            th_cookie_parser parser;
            th_cookie_parser_init(&parser, header);
            while (!th_cookie_parser_done(&parser)) {
                th_str key, value;
                th_cookie_parser_next(&parser, &key, &value);
            }
        }
        TH_BENCH_RUN_END
    }
    TH_BENCH_CASE_END

    TH_BENCH_CASE_BEGIN(parse_quoted_cookie_value, 100000)
    {
        th_str header = TH_STR("name=\"a quoted value with spaces\"");

        TH_BENCH_RUN_BEGIN
        {
            th_cookie_parser parser;
            th_cookie_parser_init(&parser, header);
            while (!th_cookie_parser_done(&parser)) {
                th_str key, value;
                th_cookie_parser_next(&parser, &key, &value);
            }
        }
        TH_BENCH_RUN_END
    }
    TH_BENCH_CASE_END

    TH_BENCH_CASE_BEGIN(parse_long_cookie_value, 10000)
    {
        th_str header = TH_STR("session=abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz0123456789");

        TH_BENCH_RUN_BEGIN
        {
            th_cookie_parser parser;
            th_cookie_parser_init(&parser, header);
            while (!th_cookie_parser_done(&parser)) {
                th_str key, value;
                th_cookie_parser_next(&parser, &key, &value);
            }
        }
        TH_BENCH_RUN_END
    }
    TH_BENCH_CASE_END
}
TH_BENCH_END
