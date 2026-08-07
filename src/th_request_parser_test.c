#include "th_request.h"
#include "th_request_parser.h"
#include "th_test.h"

#include <string.h>

TH_TEST_BEGIN(request_parser)
{
    th_request request;
    th_request_init(&request, NULL);
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
        th_part* field1 = th_request_get_part(&request, TH_STR("variable1"));
        TH_EXPECT(field1);
        TH_EXPECT(strcmp(th_part_name(field1), "variable1") == 0);
        TH_EXPECT(strcmp(th_part_filename(field1), "") == 0);
        th_buffer field1_content = th_part_content(field1);
        TH_EXPECT(strncmp(field1_content.ptr, "value1", field1_content.len) == 0);

        th_part* field2 = th_request_get_part(&request, TH_STR("variable2"));
        TH_EXPECT(field2);
        th_buffer field2_content = th_part_content(field2);
        TH_EXPECT(strncmp(field2_content.ptr, "value2", field2_content.len) == 0);

        th_part* upload = th_request_get_part(&request, TH_STR("variable3"));
        TH_EXPECT(upload);
        TH_EXPECT(strcmp(th_part_filename(upload), "example.txt") == 0);
        TH_EXPECT(strcmp(th_part_content_type(upload), "text/plain") == 0);
        TH_EXPECT(strcmp(th_part_name(upload), "variable3") == 0);
        th_buffer upload_data = th_part_content(upload);
        TH_EXPECT(strncmp(upload_data.ptr, "Hello File", upload_data.len) == 0);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parse_incomplete_method)
    {
        th_str data = TH_STR("GE");
        size_t parsed = 0;
        TH_EXPECT(th_request_parser_parse(&parser, &request, data, &parsed) == TH_ERR_OK);
        TH_EXPECT(parsed == 0);
        TH_EXPECT(!th_request_parser_done(&parser));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parse_incomplete_path)
    {
        th_str data = TH_STR("GET /te");
        size_t parsed = 0;
        TH_EXPECT(th_request_parser_parse(&parser, &request, data, &parsed) == TH_ERR_OK);
        TH_EXPECT(parsed == 4); // consumed "GET " only
        TH_EXPECT(!th_request_parser_done(&parser));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parse_incomplete_version)
    {
        th_str data = TH_STR("GET /test HTTP/1.1");
        size_t parsed = 0;
        TH_EXPECT(th_request_parser_parse(&parser, &request, data, &parsed) == TH_ERR_OK);
        TH_EXPECT(th_string_eq(&request.uri_path, TH_STR("/test")));
        TH_EXPECT(!th_request_parser_done(&parser));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parse_incomplete_header)
    {
        th_str data = TH_STR("GET /test HTTP/1.1\r\nHost: exam");
        size_t parsed = 0;
        TH_EXPECT(th_request_parser_parse(&parser, &request, data, &parsed) == TH_ERR_OK);
        TH_EXPECT(request.version == 1);
        TH_EXPECT(!th_request_parser_done(&parser));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parse_incomplete_body)
    {
        th_str data = TH_STR("POST /test HTTP/1.1\r\nContent-Length: 11\r\n\r\nHello");
        size_t parsed = 0;
        TH_EXPECT(th_request_parser_parse(&parser, &request, data, &parsed) == TH_ERR_OK);
        TH_EXPECT(!th_request_parser_done(&parser));
        TH_EXPECT(TH_STR_EQ(request.body, ""));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parse_request_fed_in_chunks)
    {
        th_str data = TH_STR("POST /test HTTP/1.1\r\nContent-Length: 11\r\n\r\nHello World");
        size_t first_parsed = 0;
        th_str first_chunk = th_str_substr(data, 0, 30);
        TH_EXPECT(th_request_parser_parse(&parser, &request, first_chunk, &first_parsed) == TH_ERR_OK);
        TH_EXPECT(!th_request_parser_done(&parser));

        size_t second_parsed = 0;
        th_str remainder = th_str_substr(data, first_parsed, th_str_npos);
        TH_EXPECT(th_request_parser_parse(&parser, &request, remainder, &second_parsed) == TH_ERR_OK);
        TH_EXPECT(th_request_parser_done(&parser));
        TH_EXPECT(request.method == TH_METHOD_POST);
        TH_EXPECT(th_string_eq(&request.uri_path, TH_STR("/test")));
        TH_EXPECT(TH_STR_EQ(request.body, "Hello World"));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parse_fed_byte_by_byte)
    {
        th_str data = TH_STR("GET /test HTTP/1.1\r\nHost: example.com\r\n\r\n");
        size_t consumed = 0;
        for (size_t end = 1; end <= data.len; end++) {
            size_t parsed = 0;
            th_str buffer = th_str_substr(data, consumed, end - consumed);
            TH_EXPECT(th_request_parser_parse(&parser, &request, buffer, &parsed) == TH_ERR_OK);
            consumed += parsed;
        }
        TH_EXPECT(th_request_parser_done(&parser));
        TH_EXPECT(request.method == TH_METHOD_GET);
        TH_EXPECT(th_string_eq(&request.uri_path, TH_STR("/test")));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parse_multipart_fed_in_chunks)
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
        size_t header_end = th_str_find_first(data, 0, '\n');
        header_end = th_str_find_first(data, header_end + 1, '\n');
        header_end = th_str_find_first(data, header_end + 1, '\n') + 1; // through the blank line after headers

        size_t first_parsed = 0;
        th_str first_chunk = th_str_substr(data, 0, header_end);
        TH_EXPECT(th_request_parser_parse(&parser, &request, first_chunk, &first_parsed) == TH_ERR_OK);
        TH_EXPECT(!th_request_parser_done(&parser));
        TH_EXPECT(!th_request_get_part(&request, TH_STR("variable1")));

        size_t second_parsed = 0;
        th_str remainder = th_str_substr(data, first_parsed, th_str_npos);
        TH_EXPECT(th_request_parser_parse(&parser, &request, remainder, &second_parsed) == TH_ERR_OK);
        TH_EXPECT(th_request_parser_done(&parser));
        th_part* field1 = th_request_get_part(&request, TH_STR("variable1"));
        TH_EXPECT(field1);
        th_buffer field1_content = th_part_content(field1);
        TH_EXPECT(strncmp(field1_content.ptr, "value1", field1_content.len) == 0);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parse_unknown_method)
    {
        th_str data = TH_STR("FOOBAR /test HTTP/1.1\r\nHost: example.com\r\n\r\n");
        size_t parsed = 0;
        TH_EXPECT(th_request_parser_parse(&parser, &request, data, &parsed) == TH_ERR_HTTP(TH_CODE_NOT_IMPLEMENTED));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parse_bad_version_line_ending)
    {
        th_str data = TH_STR("GET /test HTTP/1.1\rXHost: example.com\r\n\r\n");
        size_t parsed = 0;
        TH_EXPECT(th_request_parser_parse(&parser, &request, data, &parsed) == TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parse_bad_header_line_ending)
    {
        th_str data = TH_STR("GET /test HTTP/1.1\r\nHost: example.com\rXConnection: close\r\n\r\n");
        size_t parsed = 0;
        TH_EXPECT(th_request_parser_parse(&parser, &request, data, &parsed) == TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parse_header_name_invalid_char)
    {
        th_str data = TH_STR("GET /test HTTP/1.1\r\nHo<st: example.com\r\n\r\n");
        size_t parsed = 0;
        TH_EXPECT(th_request_parser_parse(&parser, &request, data, &parsed) == TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parse_header_name_control_char)
    {
        char buffer[] = "GET /test HTTP/1.1\r\nHo\tst: example.com\r\n\r\n";
        size_t parsed = 0;
        TH_EXPECT(th_request_parser_parse(&parser, &request, th_str_make(buffer, sizeof(buffer) - 1), &parsed) == TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parse_version_too_short)
    {
        th_str data = TH_STR("GET /test HTTP/1\r\nHost: example.com\r\n\r\n");
        size_t parsed = 0;
        TH_EXPECT(th_request_parser_parse(&parser, &request, data, &parsed) == TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parse_version_wrong_protocol_name)
    {
        th_str data = TH_STR("GET /test FOOP/1.1\r\nHost: example.com\r\n\r\n");
        size_t parsed = 0;
        TH_EXPECT(th_request_parser_parse(&parser, &request, data, &parsed) == TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parse_version_wrong_major)
    {
        th_str data = TH_STR("GET /test HTTP/2.1\r\nHost: example.com\r\n\r\n");
        size_t parsed = 0;
        TH_EXPECT(th_request_parser_parse(&parser, &request, data, &parsed) == TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parse_version_missing_dot)
    {
        th_str data = TH_STR("GET /test HTTP/1x1\r\nHost: example.com\r\n\r\n");
        size_t parsed = 0;
        TH_EXPECT(th_request_parser_parse(&parser, &request, data, &parsed) == TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parse_version_non_digit_minor)
    {
        th_str data = TH_STR("GET /test HTTP/1.x\r\nHost: example.com\r\n\r\n");
        size_t parsed = 0;
        TH_EXPECT(th_request_parser_parse(&parser, &request, data, &parsed) == TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parse_version_extra_minor_digit_rejected)
    {
        // HTTP-version is HTTP "/" DIGIT "." DIGIT (RFC 7230): a second
        // minor-version digit makes the token malformed.
        th_str data = TH_STR("GET /test HTTP/1.10\r\nHost: example.com\r\n\r\n");
        size_t parsed = 0;
        TH_EXPECT(th_request_parser_parse(&parser, &request, data, &parsed) == TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parse_version_each_character_wrong)
    {
        // Corrupt one byte of "HTTP/1.1" at a time (using a character that's
        // invalid at every position) and confirm each is rejected.
        for (size_t i = 0; i < 8; i++) {
            char buffer[] = "GET /test HTTP/1.1\r\nHost: example.com\r\n\r\n";
            const size_t version_offset = 10; // "GET /test " is 10 bytes
            buffer[version_offset + i] = '!';
            th_request_parser_reset(&parser);
            th_request_reset(&request);
            size_t parsed = 0;
            TH_EXPECT(
                th_request_parser_parse(&parser, &request, th_str_make(buffer, sizeof(buffer) - 1), &parsed)
                == TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
        }
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parse_header_name_too_long)
    {
        char buffer[2048];
        int prefix_len = snprintf(buffer, sizeof(buffer), "GET /test HTTP/1.1\r\n");
        memset(buffer + prefix_len, 'a', 1100);
        int suffix_len = snprintf(
            buffer + prefix_len + 1100, sizeof(buffer) - (size_t)prefix_len - 1100, ": value\r\n\r\n");
        size_t total_len = (size_t)prefix_len + 1100 + (size_t)suffix_len;
        size_t parsed = 0;
        TH_EXPECT(
            th_request_parser_parse(&parser, &request, th_str_make(buffer, total_len), &parsed)
            == TH_ERR_HTTP(TH_CODE_REQUEST_HEADER_FIELDS_TOO_LARGE));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parse_get_with_body_rejected)
    {
        th_str data = TH_STR("GET /test HTTP/1.1\r\nContent-Length: 5\r\n\r\nHello");
        size_t parsed = 0;
        TH_EXPECT(th_request_parser_parse(&parser, &request, data, &parsed) == TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parse_head_with_body_rejected)
    {
        th_str data = TH_STR("HEAD /test HTTP/1.1\r\nContent-Length: 5\r\n\r\nHello");
        size_t parsed = 0;
        TH_EXPECT(th_request_parser_parse(&parser, &request, data, &parsed) == TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parse_header_value_non_printable)
    {
        char buffer[] = "GET /test HTTP/1.1\r\nHost: exam\x01ple.com\r\n\r\n";
        size_t parsed = 0;
        TH_EXPECT(
            th_request_parser_parse(&parser, &request, th_str_make(buffer, sizeof(buffer) - 1), &parsed)
            == TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
    }
    TH_TEST_CASE_END

    th_request_deinit(&request);
}
TH_TEST_END
