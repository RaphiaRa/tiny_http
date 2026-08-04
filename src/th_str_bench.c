#include "th_bench.h"
#include "th_str.h"

TH_BENCH_BEGIN(str)
{
    TH_BENCH_CASE_BEGIN(find_first_short_string, 100000)
    {
        th_str str = TH_STR("Content-Type: text/plain");

        TH_BENCH_RUN_BEGIN
        {
            th_str_find_first(str, 0, ':');
        }
        TH_BENCH_RUN_END
    }
    TH_BENCH_CASE_END

    TH_BENCH_CASE_BEGIN(find_first_4kb_match_near_end, 10000)
    {
        static char buf[4096];
        for (size_t i = 0; i < sizeof(buf); i++)
            buf[i] = 'a' + (char)(i % 26);
        buf[sizeof(buf) - 1] = ';';
        th_str str = th_str_make(buf, sizeof(buf));

        TH_BENCH_RUN_BEGIN
        {
            th_str_find_first(str, 0, ';');
        }
        TH_BENCH_RUN_END
    }
    TH_BENCH_CASE_END

    TH_BENCH_CASE_BEGIN(find_first_4kb_no_match, 10000)
    {
        static char buf[4096];
        for (size_t i = 0; i < sizeof(buf); i++)
            buf[i] = 'a' + (char)(i % 26);
        th_str str = th_str_make(buf, sizeof(buf));

        TH_BENCH_RUN_BEGIN
        {
            th_str_find_first(str, 0, ';');
        }
        TH_BENCH_RUN_END
    }
    TH_BENCH_CASE_END

    TH_BENCH_CASE_BEGIN(find_first_of_short_string, 1000000)
    {
        th_str str = TH_STR("name=\"a\"");

        TH_BENCH_RUN_BEGIN
        {
            th_str_find_first_of(str, 0, "=; ");
        }
        TH_BENCH_RUN_END
    }
    TH_BENCH_CASE_END
}
TH_BENCH_END
