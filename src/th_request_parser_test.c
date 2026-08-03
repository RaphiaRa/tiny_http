#include "th_request.h"
#include "th_request_parser.h"
#include "th_test.h"

#include <string.h>

TH_TEST_BEGIN(request_parser)
{
    th_request request;
    th_request_init(&request, NULL, NULL, NULL);
    th_request_parser parser;
    th_request_parser_init(&parser);

    TH_TEST_CASE_BEGIN(parse_path_and_header)
    {
        th_str data = TH_STR("GET /test HTTP/1.1\r\nHost: example.com\r\n\r\n");
        size_t parsed = 0;
        TH_EXPECT(th_request_parser_parse(&parser, &request, data, &parsed) == TH_ERR_OK);
        TH_EXPECT(parsed == data.len);
        TH_EXPECT(request.method == TH_METHOD_GET);
        TH_EXPECT(th_string_eq(&request.uri_path, TH_STR("/test")));
        TH_EXPECT(request.version == 1);
        TH_EXPECT(TH_STR_EQ(request.body, ""));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parse_path_and_query)
    {
        th_str data = TH_STR("GET /test?key1=value1&key2=value2 HTTP/1.1\r\nHost: example.com\r\n\r\n");
        size_t parsed = 0;
        TH_EXPECT(th_request_parser_parse(&parser, &request, data, &parsed) == TH_ERR_OK);
        TH_EXPECT(parsed == data.len);
        TH_EXPECT(request.method == TH_METHOD_GET);
        TH_EXPECT(th_string_eq(&request.uri_path, TH_STR("/test")));
        TH_EXPECT(request.version == 1);
        TH_EXPECT(TH_STR_EQ(th_request_get_queryvar(&request, TH_STR("key1")), "value1"));
        TH_EXPECT(TH_STR_EQ(th_request_get_queryvar(&request, TH_STR("key2")), "value2"));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parse_path_and_body)
    {
        th_str data = TH_STR("POST /test HTTP/1.1\r\nHost: example.com\r\nContent-Length: 11\r\n\r\nHello World");
        size_t parsed = 0;
        TH_EXPECT(th_request_parser_parse(&parser, &request, data, &parsed) == TH_ERR_OK);
        TH_EXPECT(parsed == data.len);
        TH_EXPECT(request.method == TH_METHOD_POST);
        TH_EXPECT(th_string_eq(&request.uri_path, TH_STR("/test")));
        TH_EXPECT(request.version == 1);
        TH_EXPECT(TH_STR_EQ(request.body, "Hello World"));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parse_bad_content)
    {
        th_str data = TH_STR("GET /index.php?variable=..%2F..%2F..%2F..%2F..%2F..%2F..%2F%2Fetc HTTP/1.1\r\n"
                             "Host: localhost\r\nConnection: Keep-Alive\r\n"
                             "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/74.0.3729.169 Safari/537.36\r\n\r\n"
                             "36 (KHTML, like Gecko) Chrome/74.0.3729.169 Safari/537.36\r\n"
                             "Host: localhost\r\n"
                             "Connection: Keep-Alive\r\n\r\n");
        size_t parsed = 0;
        TH_EXPECT(th_request_parser_parse(&parser, &request, data, &parsed) == TH_ERR_OK);
        TH_EXPECT(request.method == TH_METHOD_GET);
        TH_EXPECT(th_string_eq(&request.uri_path, TH_STR("/index.php")));
        TH_EXPECT(TH_STR_EQ(th_request_get_queryvar(&request, TH_STR("variable")), "../../../../../../..//etc"));
        TH_EXPECT(request.version == 1);
        TH_EXPECT(TH_STR_EQ(request.body, ""));
        TH_EXPECT(parsed == 248);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parse_bad_query_encoding)
    {
        th_str data = TH_STR("GET /index.php?variable=h%2411%7C%7B%7D+W_%26%26%21rld%7E%7E%7E%7Eh%2411%7C%7B%7D+W_%26%26%21rld%7E%7E%7E%rr HTTP/1.1\r\n"
                             "Host: localhost\r\nConnection: Keep-Alive\r\n"
                             "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/74.0.3729.169 Safari/537.36\r\n\r\n");
        size_t parsed = 0;
        TH_EXPECT(th_request_parser_parse(&parser, &request, data, &parsed) == TH_ERR_OK);
        TH_EXPECT(request.method == TH_METHOD_GET);
        TH_EXPECT(th_string_eq(&request.uri_path, TH_STR("/index.php")));
        TH_EXPECT(TH_STR_EQ(th_request_get_queryvar(&request, TH_STR("variable")), ""));
        TH_EXPECT(request.version == 1);
        TH_EXPECT(TH_STR_EQ(request.body, ""));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parse_empty_query_key)
    {
        th_str data = TH_STR("GET /index.php?=qwertqwertqwertqwertqwertqwertqwertqwertqwertqwertqwetqwert HTTP/1.1\r\nHost: localhost\r\nConnection: Keep-Alive\r\n\r\n");
        size_t parsed = 0;
        TH_EXPECT(th_request_parser_parse(&parser, &request, data, &parsed) == TH_ERR_OK);
        TH_EXPECT(request.method == TH_METHOD_GET);
        TH_EXPECT(th_string_eq(&request.uri_path, TH_STR("/index.php")));
        TH_EXPECT(TH_STR_EQ(th_request_get_queryvar(&request, TH_STR("")), "qwertqwertqwertqwertqwertqwertqwertqwertqwertqwertqwetqwert"));
        TH_EXPECT(request.version == 1);
        TH_EXPECT(TH_STR_EQ(request.body, ""));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(request_parser_empty_header)
    {
        th_str data = TH_STR("POST / HTTP/1.1\r\n: 0080\r\nUser-Agent:81.0\r\nAccept: */*\r\nContent-Length: 0\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\n");
        size_t parsed = 0;
        TH_EXPECT(th_request_parser_parse(&parser, &request, data, &parsed) == TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parse_bad_form_encoding)
    {
        char buffer[] = "POST / HTTP/1.1\r\nContent-Length: 3\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\n=n";
        buffer[sizeof(buffer) - 1] = '%';
        size_t parsed = 0;
        TH_EXPECT(th_request_parser_parse(&parser, &request, th_str_make(buffer, sizeof(buffer)), &parsed) == TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parse_cookies)
    {
        th_str data = TH_STR("GET /test HTTP/1.1\r\nHost: example.com\r\nCookie: name1=value1; name2=value2\r\n\r\n");
        size_t parsed = 0;
        TH_EXPECT(th_request_parser_parse(&parser, &request, data, &parsed) == TH_ERR_OK);
        TH_EXPECT(parsed == data.len);
        TH_EXPECT(strcmp(th_find_cookie(&request, "name1"), "value1") == 0);
        TH_EXPECT(strcmp(th_find_cookie(&request, "name2"), "value2") == 0);
        TH_EXPECT(th_find_cookie(&request, "nonexistent") == NULL);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parse_cookie_with_extra_whitespace)
    {
        th_str data = TH_STR("GET /test HTTP/1.1\r\nHost: example.com\r\nCookie:   name  =  value  \r\n\r\n");
        size_t parsed = 0;
        TH_EXPECT(th_request_parser_parse(&parser, &request, data, &parsed) == TH_ERR_OK);
        TH_EXPECT(parsed == data.len);
        TH_EXPECT(strcmp(th_find_cookie(&request, "name"), "value") == 0);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parse_quoted_cookie_value)
    {
        // RFC 6265: a cookie-value may be wrapped in a single pair of
        // DQUOTEs, which are wire-format framing, not part of the value.
        th_str data = TH_STR("GET /test HTTP/1.1\r\nHost: example.com\r\nCookie: name=\"quoted value\"\r\n\r\n");
        size_t parsed = 0;
        TH_EXPECT(th_request_parser_parse(&parser, &request, data, &parsed) == TH_ERR_OK);
        TH_EXPECT(parsed == data.len);
        TH_EXPECT(strcmp(th_find_cookie(&request, "name"), "quoted value") == 0);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parse_bad_cookie_missing_equals)
    {
        th_str data = TH_STR("GET /test HTTP/1.1\r\nHost: example.com\r\nCookie: not_a_valid_cookie\r\n\r\n");
        size_t parsed = 0;
        TH_EXPECT(th_request_parser_parse(&parser, &request, data, &parsed) == TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parse_multipart_form_data)
    {
        th_str data = TH_STR("POST / HTTP/1.1\r\nContent-Length: 472\r\n"
                             "Content-Type: multipart/form-data; boundary=---------------------------9051914041544843365972754266\r\n\r\n"
                             "-----------------------------9051914041544843365972754266\r\n"
                             "Content-Disposition: form-data; name=\"variable1\"\r\n\r\n"
                             "value1\r\n"
                             "-----------------------------9051914041544843365972754266\r\n"
                             "Content-Disposition: form-data; name=\"variable2\"\r\n\r\n"
                             "value2\r\n"
                             "-----------------------------9051914041544843365972754266\r\n"
                             "Content-Disposition: form-data; name=\"variable3\"; filename=\"example.txt\"\r\n"
                             "Content-Type: text/plain\r\n\r\n"
                             "Hello File\r\n"
                             "-----------------------------9051914041544843365972754266--\r\n");
        size_t parsed = 0;
        TH_EXPECT(th_request_parser_parse(&parser, &request, data, &parsed) == TH_ERR_OK);
        TH_EXPECT(request.method == TH_METHOD_POST);
        TH_EXPECT(th_string_eq(&request.uri_path, TH_STR("/")));
        TH_EXPECT(request.version == 1);
        TH_EXPECT(TH_STR_EQ(th_request_get_formvar(&request, TH_STR("variable1")), "value1"));
        TH_EXPECT(TH_STR_EQ(th_request_get_formvar(&request, TH_STR("variable2")), "value2"));
        th_upload* upload = th_request_get_upload(&request, TH_STR("variable3"));
        TH_EXPECT(upload);
        TH_EXPECT(strcmp(th_upload_get_info(upload).filename, "example.txt") == 0);
        TH_EXPECT(strcmp(th_upload_get_info(upload).content_type, "text/plain") == 0);
        TH_EXPECT(strcmp(th_upload_get_info(upload).name, "variable3") == 0);
        th_buffer upload_data = th_upload_get_data(upload);
        TH_EXPECT(strncmp(upload_data.ptr, "Hello File", upload_data.len) == 0);
    }
    TH_TEST_CASE_END

    th_request_deinit(&request);
}
TH_TEST_END
