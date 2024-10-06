#include "th_request.h"
#include "th_request_parser.h"
#include "th_test.h"

#include <string.h>

TH_TEST_BEGIN(request_parser)
{
    TH_TEST_CASE_BEGIN(parse_path_and_header)
    {
        th_request request;
        th_request_init(&request, NULL);
        th_request_parser parser;
        th_request_parser_init(&parser);
        th_string data = TH_STRING("GET /test HTTP/1.1\r\nHost: example.com\r\n\r\n");
        size_t parsed = 0;
        TH_EXPECT(th_request_parser_parse(&parser, &request, data, &parsed) == TH_ERR_OK);
        TH_EXPECT(parsed == data.len);
        TH_EXPECT(request.method == TH_METHOD_GET);
        TH_EXPECT(th_heap_string_eq(&request.uri_path, TH_STRING("/test")));
        TH_EXPECT(request.version == 1);
        TH_EXPECT(TH_STRING_EQ(request.body, ""));
        th_request_deinit(&request);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parse_path_and_query)
    {
        th_request request;
        th_request_init(&request, NULL);
        th_request_parser parser;
        th_request_parser_init(&parser);
        th_string data = TH_STRING("GET /test?key1=value1&key2=value2 HTTP/1.1\r\nHost: example.com\r\n\r\n");
        size_t parsed = 0;
        TH_EXPECT(th_request_parser_parse(&parser, &request, data, &parsed) == TH_ERR_OK);
        TH_EXPECT(parsed == data.len);
        TH_EXPECT(request.method == TH_METHOD_GET);
        TH_EXPECT(th_heap_string_eq(&request.uri_path, TH_STRING("/test")));
        TH_EXPECT(request.version == 1);
        const char* qv = NULL;
        TH_EXPECT((qv = th_try_get_query_param(&request, "key1")) != NULL && strcmp(qv, "value1") == 0);
        TH_EXPECT((qv = th_try_get_query_param(&request, "key2")) != NULL && strcmp(qv, "value2") == 0);
        th_request_deinit(&request);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parse_path_and_body)
    {
        th_request request;
        th_request_init(&request, NULL);
        th_request_parser parser;
        th_request_parser_init(&parser);
        th_string data = TH_STRING("POST /test HTTP/1.1\r\nHost: example.com\r\nContent-Length: 11\r\n\r\nHello World");
        size_t parsed = 0;
        TH_EXPECT(th_request_parser_parse(&parser, &request, data, &parsed) == TH_ERR_OK);
        TH_EXPECT(parsed == data.len);
        TH_EXPECT(request.method == TH_METHOD_POST);
        TH_EXPECT(th_heap_string_eq(&request.uri_path, TH_STRING("/test")));
        TH_EXPECT(request.version == 1);
        TH_EXPECT(TH_STRING_EQ(request.body, "Hello World"));
        th_request_deinit(&request);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parse_bad)
    {
        th_request request;
        th_request_init(&request, NULL);
        th_request_parser parser;
        th_request_parser_init(&parser);
        th_string data = TH_STRING("GET /index.php?chemin=..%2F..%2F..%2F..%2F..%2F..%2F..%2F%2Fetc HTTP/1.1\r\n"
                                   "Host: localhost\r\nConnection: Keep-Alive\r\n"
                                   "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/74.0.3729.169 Safari/537.36\r\n\r\n"
                                   "36 (KHTML, like Gecko) Chrome/74.0.3729.169 Safari/537.36\r\n"
                                   "Host: localhost\r\n"
                                   "Connection: Keep-Alive\r\n\r\n");
        size_t parsed = 0;
        TH_EXPECT(th_request_parser_parse(&parser, &request, data, &parsed) == TH_ERR_OK);
        TH_EXPECT(request.method == TH_METHOD_GET);
        TH_EXPECT(th_heap_string_eq(&request.uri_path, TH_STRING("/index.php")));
        const char* qv = NULL;
        TH_EXPECT((qv = th_try_get_query_param(&request, "chemin")) != NULL && strcmp(qv, "../../../../../../..//etc") == 0);
        TH_EXPECT(request.version == 1);
        TH_EXPECT(TH_STRING_EQ(request.body, ""));
        TH_EXPECT(parsed == 246);
        th_request_deinit(&request);
    }
    TH_TEST_CASE_END
}
TH_TEST_END
