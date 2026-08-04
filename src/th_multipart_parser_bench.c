#include "th_bench.h"
#include "th_multipart_parser.h"

#include <stdio.h>

TH_LOCAL(size_t)
th_multipart_bench_build_large_body(char* buf, size_t buf_size, size_t content_len, bool with_content_length)
{
    size_t pos = 0;
    pos += (size_t)snprintf(buf + pos, buf_size - pos,
                            "--boundary\r\n"
                            "Content-Disposition: form-data; name=\"file\"; filename=\"a.bin\"\r\n"
                            "Content-Type: application/octet-stream\r\n");
    if (with_content_length)
        pos += (size_t)snprintf(buf + pos, buf_size - pos, "Content-Length: %zu\r\n", content_len);
    pos += (size_t)snprintf(buf + pos, buf_size - pos, "\r\n");
    for (size_t i = 0; i < content_len; i++)
        buf[pos++] = (char)('a' + (int)(i % 26));
    pos += (size_t)snprintf(buf + pos, buf_size - pos, "\r\n--boundary--\r\n");
    return pos;
}

TH_BENCH_BEGIN(multipart_parser)
{
    TH_BENCH_CASE_BEGIN(parse_boundary_from_content_type, 100000)
    {
        th_str content_type = TH_STR("multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW");

        TH_BENCH_RUN_BEGIN
        {
            th_str boundary;
            th_multipart_parser_boundary(content_type, &boundary);
        }
        TH_BENCH_RUN_END
    }
    TH_BENCH_CASE_END

    TH_BENCH_CASE_BEGIN(parse_single_small_field, 100000)
    {
        th_str body = TH_STR("--boundary\r\n"
                             "Content-Disposition: form-data; name=\"field1\"\r\n\r\n"
                             "value1\r\n"
                             "--boundary--\r\n");

        TH_BENCH_RUN_BEGIN
        {
            th_multipart_parser parser;
            th_multipart_parser_init(&parser, body, TH_STR("boundary"));
            while (!th_multipart_parser_done(&parser)) {
                th_multipart_part part;
                th_multipart_parser_next(&parser, &part);
            }
        }
        TH_BENCH_RUN_END
    }
    TH_BENCH_CASE_END

    TH_BENCH_CASE_BEGIN(parse_ten_fields, 10000)
    {
        th_str body = TH_STR("--boundary\r\n"
                             "Content-Disposition: form-data; name=\"a\"\r\n\r\n1\r\n"
                             "--boundary\r\n"
                             "Content-Disposition: form-data; name=\"b\"\r\n\r\n2\r\n"
                             "--boundary\r\n"
                             "Content-Disposition: form-data; name=\"c\"\r\n\r\n3\r\n"
                             "--boundary\r\n"
                             "Content-Disposition: form-data; name=\"d\"\r\n\r\n4\r\n"
                             "--boundary\r\n"
                             "Content-Disposition: form-data; name=\"e\"\r\n\r\n5\r\n"
                             "--boundary\r\n"
                             "Content-Disposition: form-data; name=\"f\"\r\n\r\n6\r\n"
                             "--boundary\r\n"
                             "Content-Disposition: form-data; name=\"g\"\r\n\r\n7\r\n"
                             "--boundary\r\n"
                             "Content-Disposition: form-data; name=\"h\"\r\n\r\n8\r\n"
                             "--boundary\r\n"
                             "Content-Disposition: form-data; name=\"i\"\r\n\r\n9\r\n"
                             "--boundary\r\n"
                             "Content-Disposition: form-data; name=\"j\"\r\n\r\n10\r\n"
                             "--boundary--\r\n");

        TH_BENCH_RUN_BEGIN
        {
            th_multipart_parser parser;
            th_multipart_parser_init(&parser, body, TH_STR("boundary"));
            while (!th_multipart_parser_done(&parser)) {
                th_multipart_part part;
                th_multipart_parser_next(&parser, &part);
            }
        }
        TH_BENCH_RUN_END
    }
    TH_BENCH_CASE_END

    TH_BENCH_CASE_BEGIN(parse_4kb_file_via_content_length, 10000)
    {
        char buf[4096 + 256];
        size_t len = th_multipart_bench_build_large_body(buf, sizeof(buf), 4096, true);
        th_str body = th_str_make(buf, len);

        TH_BENCH_RUN_BEGIN
        {
            th_multipart_parser parser;
            th_multipart_parser_init(&parser, body, TH_STR("boundary"));
            while (!th_multipart_parser_done(&parser)) {
                th_multipart_part part;
                th_multipart_parser_next(&parser, &part);
            }
        }
        TH_BENCH_RUN_END
    }
    TH_BENCH_CASE_END

    TH_BENCH_CASE_BEGIN(parse_4kb_file_via_boundary_scan, 10000)
    {
        char buf[4096 + 256];
        size_t len = th_multipart_bench_build_large_body(buf, sizeof(buf), 4096, false);
        th_str body = th_str_make(buf, len);

        TH_BENCH_RUN_BEGIN
        {
            th_multipart_parser parser;
            th_multipart_parser_init(&parser, body, TH_STR("boundary"));
            while (!th_multipart_parser_done(&parser)) {
                th_multipart_part part;
                th_multipart_parser_next(&parser, &part);
            }
        }
        TH_BENCH_RUN_END
    }
    TH_BENCH_CASE_END
}
TH_BENCH_END
