#include "th_bench.h"
#include "th_filepath.h"

TH_BENCH_BEGIN(filepath)
{
    TH_BENCH_CASE_BEGIN(init_clean_short_path, 1000000)
    {
        th_str input = TH_STR("uploads/photo.jpg");
        th_filepath path;

        TH_BENCH_RUN_BEGIN
        {
            th_filepath_init(&path, input);
        }
        TH_BENCH_RUN_END
    }
    TH_BENCH_CASE_END

    TH_BENCH_CASE_BEGIN(init_needs_normalization, 1000000)
    {
        th_str input = TH_STR("uploads/./foo//bar/./photo.jpg");
        th_filepath path;

        TH_BENCH_RUN_BEGIN
        {
            th_filepath_init(&path, input);
        }
        TH_BENCH_RUN_END
    }
    TH_BENCH_CASE_END

    TH_BENCH_CASE_BEGIN(init_long_nested_path, 500000)
    {
        th_str input = TH_STR("a/b/c/d/e/f/g/h/i/j/k/l/m/n/o/p/q/r/s/t/photo.jpg");
        th_filepath path;

        TH_BENCH_RUN_BEGIN
        {
            th_filepath_init(&path, input);
        }
        TH_BENCH_RUN_END
    }
    TH_BENCH_CASE_END

    TH_BENCH_CASE_BEGIN(init_rejects_traversal, 1000000)
    {
        th_str input = TH_STR("../../../etc/passwd");
        th_filepath path;

        TH_BENCH_RUN_BEGIN
        {
            th_filepath_init(&path, input);
        }
        TH_BENCH_RUN_END
    }
    TH_BENCH_CASE_END
}
TH_BENCH_END
