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
        TH_EXPECT(TH_STRING_EQ(th_request_get_queryvar(&request, TH_STRING("key1")), "value1"));
        TH_EXPECT(TH_STRING_EQ(th_request_get_queryvar(&request, TH_STRING("key2")), "value2"));
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
    TH_TEST_CASE_BEGIN(parse_bad_content)
    {
        th_request request;
        th_request_init(&request, NULL);
        th_request_parser parser;
        th_request_parser_init(&parser);
        th_string data = TH_STRING("GET /index.php?variable=..%2F..%2F..%2F..%2F..%2F..%2F..%2F%2Fetc HTTP/1.1\r\n"
                                   "Host: localhost\r\nConnection: Keep-Alive\r\n"
                                   "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/74.0.3729.169 Safari/537.36\r\n\r\n"
                                   "36 (KHTML, like Gecko) Chrome/74.0.3729.169 Safari/537.36\r\n"
                                   "Host: localhost\r\n"
                                   "Connection: Keep-Alive\r\n\r\n");
        size_t parsed = 0;
        TH_EXPECT(th_request_parser_parse(&parser, &request, data, &parsed) == TH_ERR_OK);
        TH_EXPECT(request.method == TH_METHOD_GET);
        TH_EXPECT(th_heap_string_eq(&request.uri_path, TH_STRING("/index.php")));
        TH_EXPECT(TH_STRING_EQ(th_request_get_queryvar(&request, TH_STRING("variable")), "../../../../../../..//etc"));
        TH_EXPECT(request.version == 1);
        TH_EXPECT(TH_STRING_EQ(request.body, ""));
        TH_EXPECT(parsed == 248);
        th_request_deinit(&request);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parse_bad_query_encoding)
    {
        th_request request;
        th_request_init(&request, NULL);
        th_request_parser parser;
        th_request_parser_init(&parser);
        th_string data = TH_STRING("GET /index.php?variable=h%2411%7C%7B%7D+W_%26%26%21rld%7E%7E%7E%7Eh%2411%7C%7B%7D+W_%26%26%21rld%7E%7E%7E%rr HTTP/1.1\r\n"
                                   "Host: localhost\r\nConnection: Keep-Alive\r\n"
                                   "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/74.0.3729.169 Safari/537.36\r\n\r\n");
        size_t parsed = 0;
        TH_EXPECT(th_request_parser_parse(&parser, &request, data, &parsed) == TH_ERR_OK);
        TH_EXPECT(request.method == TH_METHOD_GET);
        TH_EXPECT(th_heap_string_eq(&request.uri_path, TH_STRING("/index.php")));
        TH_EXPECT(TH_STRING_EQ(th_request_get_queryvar(&request, TH_STRING("variable")), ""));
        TH_EXPECT(request.version == 1);
        TH_EXPECT(TH_STRING_EQ(request.body, ""));
        th_request_deinit(&request);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parse_empty_query_key)
    {
        th_request request;
        th_request_init(&request, NULL);
        th_request_parser parser;
        th_request_parser_init(&parser);
        th_string data = TH_STRING("GET /index.php?=qwertqwertqwertqwertqwertqwertqwertqwertqwertqwertqwetqwert HTTP/1.1\r\nHost: localhost\r\nConnection: Keep-Alive\r\n\r\n");
        size_t parsed = 0;
        TH_EXPECT(th_request_parser_parse(&parser, &request, data, &parsed) == TH_ERR_OK);
        TH_EXPECT(request.method == TH_METHOD_GET);
        TH_EXPECT(th_heap_string_eq(&request.uri_path, TH_STRING("/index.php")));
        TH_EXPECT(TH_STRING_EQ(th_request_get_queryvar(&request, TH_STRING("")), "qwertqwertqwertqwertqwertqwertqwertqwertqwertqwertqwetqwert"));
        TH_EXPECT(request.version == 1);
        TH_EXPECT(TH_STRING_EQ(request.body, ""));
        th_request_deinit(&request);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(request_parser_empty_header)
    {
        th_request request;
        th_request_init(&request, NULL);
        th_request_parser parser;
        th_request_parser_init(&parser);
        th_string data = TH_STRING("POST / HTTP/1.1\r\n: 0080\r\nUser-Agent:81.0\r\nAccept: */*\r\nContent-Length: 0\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\n");
        size_t parsed = 0;
        TH_EXPECT(th_request_parser_parse(&parser, &request, data, &parsed) == TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
        th_request_deinit(&request);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parse_bad_form_encoding)
    {
        th_request request;
        th_request_init(&request, NULL);
        th_request_parser parser;
        th_request_parser_init(&parser);
        char buffer[] = "POST / HTTP/1.1\r\nContent-Length: 3\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\n=n";
        buffer[sizeof(buffer) - 1] = '%';
        size_t parsed = 0;
        TH_EXPECT(th_request_parser_parse(&parser, &request, th_string_make(buffer, sizeof(buffer)), &parsed) == TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
        th_request_deinit(&request);
    }
    TH_TEST_CASE_END
}
TH_TEST_END
