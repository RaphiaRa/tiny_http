#include "th_bench.h"
#include "th_str.h"

static char th_bench_needle_near_end_buf[4096];
static char th_bench_no_match_buf[4096];

TH_BENCH_BEGIN(str)
{
    for (size_t i = 0; i < sizeof(th_bench_needle_near_end_buf); i++)
        th_bench_needle_near_end_buf[i] = 'a' + (char)(i % 26);
    th_bench_needle_near_end_buf[sizeof(th_bench_needle_near_end_buf) - 1] = ';';
    th_str needle_near_end_str = th_str_make(th_bench_needle_near_end_buf, sizeof(th_bench_needle_near_end_buf));

    for (size_t i = 0; i < sizeof(th_bench_no_match_buf); i++)
        th_bench_no_match_buf[i] = 'a' + (char)(i % 26);
    th_str no_match_str = th_str_make(th_bench_no_match_buf, sizeof(th_bench_no_match_buf));

    TH_BENCH_CASE_BEGIN(find_first_short_string, 100000)
    {
        th_str_find_first(TH_STR("Content-Type: text/plain"), 0, ':');
    }
    TH_BENCH_CASE_END

    TH_BENCH_CASE_BEGIN(find_first_4kb_match_near_end, 10000)
    {
        th_str_find_first(needle_near_end_str, 0, ';');
    }
    TH_BENCH_CASE_END

    TH_BENCH_CASE_BEGIN(find_first_4kb_no_match, 10000)
    {
        th_str_find_first(no_match_str, 0, ';');
    }
    TH_BENCH_CASE_END

    TH_BENCH_CASE_BEGIN(find_first_of_short_string, 1000000)
    {
        th_str_find_first_of(TH_STR("name=\"a\""), 0, "=; ");
    }
    TH_BENCH_CASE_END
}
TH_BENCH_END
