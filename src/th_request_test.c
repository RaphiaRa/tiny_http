#include "th_request.h"
#include "th_test.h"

#include <string.h>

TH_TEST_BEGIN(request)
{
    th_request request;
    th_request_init(&request, th_default_allocator_get());

    TH_TEST_CASE_BEGIN(request_init_is_empty)
    {
        TH_EXPECT(strcmp(th_get_path(&request), "") == 0);
        TH_EXPECT(strcmp(th_get_query(&request), "") == 0);
        TH_EXPECT(th_get_body(&request).len == 0);
        TH_EXPECT(th_find_header(&request, "Host") == NULL);
        TH_EXPECT(th_find_cookie(&request, "session") == NULL);
        TH_EXPECT(th_find_queryvar(&request, "q") == NULL);
        TH_EXPECT(th_find_formvar(&request, "f") == NULL);
        TH_EXPECT(th_find_pathvar(&request, "id") == NULL);
        TH_EXPECT(th_find_part(&request, "file") == NULL);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(request_set_uri_path_and_query)
    {
        TH_EXPECT(th_request_set_uri_path(&request, TH_STR("/a/b")) == TH_ERR_OK);
        TH_EXPECT(th_request_set_uri_query(&request, TH_STR("x=1")) == TH_ERR_OK);
        TH_EXPECT(strcmp(th_get_path(&request), "/a/b") == 0);
        TH_EXPECT(strcmp(th_get_query(&request), "x=1") == 0);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(request_set_method_and_version)
    {
        th_request_set_method(&request, TH_METHOD_POST);
        th_request_set_version(&request, TH_HTTP_1_0);
        TH_EXPECT(th_get_method(&request) == TH_METHOD_POST);
        TH_EXPECT(th_get_version(&request) == TH_HTTP_1_0);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(request_set_body)
    {
        th_request_set_body(&request, TH_STR("hello"));
        th_buffer body = th_get_body(&request);
        TH_EXPECT(body.len == 5);
        TH_EXPECT(memcmp(body.ptr, "hello", 5) == 0);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(request_add_and_find_header)
    {
        TH_EXPECT(th_request_add_header(&request, TH_STR("Host"), TH_STR("example.com")) == TH_ERR_OK);
        const char* value = th_find_header(&request, "Host");
        TH_EXPECT(value != NULL);
        TH_EXPECT(strcmp(value, "example.com") == 0);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(request_find_header_missing_returns_null)
    {
        TH_EXPECT(th_request_add_header(&request, TH_STR("Host"), TH_STR("example.com")) == TH_ERR_OK);
        TH_EXPECT(th_find_header(&request, "Accept") == NULL);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(request_find_header_does_not_match_key_that_is_only_a_prefix_of_the_search_term)
    {
        TH_EXPECT(th_request_add_header(&request, TH_STR("Ho"), TH_STR("wrong")) == TH_ERR_OK);
        TH_EXPECT(th_find_header(&request, "Host") == NULL);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(request_header_iter_visits_all_headers)
    {
        TH_EXPECT(th_request_add_header(&request, TH_STR("Host"), TH_STR("example.com")) == TH_ERR_OK);
        TH_EXPECT(th_request_add_header(&request, TH_STR("Accept"), TH_STR("*/*")) == TH_ERR_OK);

        th_iter it = th_header_iter(&request);
        size_t count = 0;
        bool found_host = false;
        bool found_accept = false;
        do {
            count++;
            if (strcmp(th_key(&it), "Host") == 0 && strcmp(th_cval(&it), "example.com") == 0)
                found_host = true;
            if (strcmp(th_key(&it), "Accept") == 0 && strcmp(th_cval(&it), "*/*") == 0)
                found_accept = true;
        } while (th_next(&it));

        TH_EXPECT(count == 2);
        TH_EXPECT(found_host);
        TH_EXPECT(found_accept);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(request_header_iter_on_empty_request_yields_no_elements)
    {
        th_iter it = th_header_iter(&request);
        TH_EXPECT(it.ptr == it.end);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(request_add_and_find_cookie)
    {
        TH_EXPECT(th_request_add_cookie(&request, TH_STR("session"), TH_STR("abc123")) == TH_ERR_OK);
        const char* value = th_find_cookie(&request, "session");
        TH_EXPECT(value != NULL);
        TH_EXPECT(strcmp(value, "abc123") == 0);
        TH_EXPECT(th_find_cookie(&request, "other") == NULL);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(request_cookie_iter_visits_all_cookies)
    {
        TH_EXPECT(th_request_add_cookie(&request, TH_STR("a"), TH_STR("1")) == TH_ERR_OK);
        TH_EXPECT(th_request_add_cookie(&request, TH_STR("b"), TH_STR("2")) == TH_ERR_OK);

        th_iter it = th_cookie_iter(&request);
        size_t count = 0;
        do {
            count++;
        } while (th_next(&it));
        TH_EXPECT(count == 2);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(request_add_and_find_queryvar_url_decodes)
    {
        // th_request_add_queryvar url-decodes both key and value.
        TH_EXPECT(th_request_add_queryvar(&request, TH_STR("na%20me"), TH_STR("john+doe")) == TH_ERR_OK);
        const char* value = th_find_queryvar(&request, "na me");
        TH_EXPECT(value != NULL);
        TH_EXPECT(strcmp(value, "john doe") == 0);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(request_add_queryvar_rejects_invalid_percent_encoding)
    {
        TH_EXPECT(th_request_add_queryvar(&request, TH_STR("key"), TH_STR("bad%")) != TH_ERR_OK);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(request_queryvar_iter_visits_all_queryvars)
    {
        TH_EXPECT(th_request_add_queryvar(&request, TH_STR("a"), TH_STR("1")) == TH_ERR_OK);
        TH_EXPECT(th_request_add_queryvar(&request, TH_STR("b"), TH_STR("2")) == TH_ERR_OK);

        th_iter it = th_queryvar_iter(&request);
        size_t count = 0;
        do {
            count++;
        } while (th_next(&it));
        TH_EXPECT(count == 2);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(request_clear_queryvars_removes_all)
    {
        TH_EXPECT(th_request_add_queryvar(&request, TH_STR("a"), TH_STR("1")) == TH_ERR_OK);
        th_request_clear_queryvars(&request);
        TH_EXPECT(th_find_queryvar(&request, "a") == NULL);
        th_iter it = th_queryvar_iter(&request);
        TH_EXPECT(it.ptr == it.end);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(request_add_and_find_formvar_url_decodes)
    {
        TH_EXPECT(th_request_add_formvar(&request, TH_STR("a+b"), TH_STR("c%20d")) == TH_ERR_OK);
        const char* value = th_find_formvar(&request, "a b");
        TH_EXPECT(value != NULL);
        TH_EXPECT(strcmp(value, "c d") == 0);
        TH_EXPECT(th_find_formvar(&request, "missing") == NULL);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(request_formvar_iter_visits_all_formvars)
    {
        TH_EXPECT(th_request_add_formvar(&request, TH_STR("a"), TH_STR("1")) == TH_ERR_OK);
        TH_EXPECT(th_request_add_formvar(&request, TH_STR("b"), TH_STR("2")) == TH_ERR_OK);

        th_iter it = th_formvar_iter(&request);
        size_t count = 0;
        do {
            count++;
        } while (th_next(&it));
        TH_EXPECT(count == 2);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(request_add_and_find_pathvar_does_not_url_decode)
    {
        // Unlike query/form vars, pathvars are stored verbatim.
        TH_EXPECT(th_request_add_pathvar(&request, TH_STR("id"), TH_STR("a%20b")) == TH_ERR_OK);
        const char* value = th_find_pathvar(&request, "id");
        TH_EXPECT(value != NULL);
        TH_EXPECT(strcmp(value, "a%20b") == 0);
        TH_EXPECT(th_find_pathvar(&request, "missing") == NULL);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(request_pathvar_iter_visits_all_pathvars)
    {
        TH_EXPECT(th_request_add_pathvar(&request, TH_STR("a"), TH_STR("1")) == TH_ERR_OK);
        TH_EXPECT(th_request_add_pathvar(&request, TH_STR("b"), TH_STR("2")) == TH_ERR_OK);

        th_iter it = th_pathvar_iter(&request);
        size_t count = 0;
        do {
            count++;
        } while (th_next(&it));
        TH_EXPECT(count == 2);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(request_add_and_find_part)
    {
        TH_EXPECT(
            th_request_add_part(&request, TH_STR("file content"), TH_STR("upload"), TH_STR("a.txt"), TH_STR("text/plain"))
            == TH_ERR_OK);

        const th_part* part = th_find_part(&request, "upload");
        TH_EXPECT(part != NULL);
        TH_EXPECT(strcmp(th_part_name(part), "upload") == 0);
        TH_EXPECT(strcmp(th_part_filename(part), "a.txt") == 0);
        TH_EXPECT(strcmp(th_part_content_type(part), "text/plain") == 0);
        th_buffer content = th_part_content(part);
        TH_EXPECT(content.len == 12);
        TH_EXPECT(memcmp(content.ptr, "file content", 12) == 0);
        TH_EXPECT(th_find_part(&request, "missing") == NULL);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(request_part_iter_visits_all_parts)
    {
        TH_EXPECT(th_request_add_part(&request, TH_STR("a"), TH_STR("one"), TH_STR(""), TH_STR("")) == TH_ERR_OK);
        TH_EXPECT(th_request_add_part(&request, TH_STR("b"), TH_STR("two"), TH_STR(""), TH_STR("")) == TH_ERR_OK);

        th_iter it = th_part_iter(&request);
        size_t count = 0;
        do {
            count++;
        } while (th_next(&it));
        TH_EXPECT(count == 2);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(request_reset_clears_everything)
    {
        TH_EXPECT(th_request_set_uri_path(&request, TH_STR("/a")) == TH_ERR_OK);
        TH_EXPECT(th_request_set_uri_query(&request, TH_STR("x=1")) == TH_ERR_OK);
        TH_EXPECT(th_request_add_header(&request, TH_STR("Host"), TH_STR("example.com")) == TH_ERR_OK);
        TH_EXPECT(th_request_add_cookie(&request, TH_STR("session"), TH_STR("abc")) == TH_ERR_OK);
        TH_EXPECT(th_request_add_queryvar(&request, TH_STR("a"), TH_STR("1")) == TH_ERR_OK);
        TH_EXPECT(th_request_add_formvar(&request, TH_STR("b"), TH_STR("2")) == TH_ERR_OK);
        TH_EXPECT(th_request_add_pathvar(&request, TH_STR("c"), TH_STR("3")) == TH_ERR_OK);
        TH_EXPECT(th_request_add_part(&request, TH_STR("data"), TH_STR("f"), TH_STR(""), TH_STR("")) == TH_ERR_OK);
        th_request_set_body(&request, TH_STR("body"));
        th_request_set_method(&request, TH_METHOD_POST);
        th_request_set_version(&request, TH_HTTP_1_1);
        request.close = true;

        th_request_reset(&request);

        TH_EXPECT(strcmp(th_get_path(&request), "") == 0);
        TH_EXPECT(strcmp(th_get_query(&request), "") == 0);
        TH_EXPECT(th_get_body(&request).len == 0);
        TH_EXPECT(th_get_version(&request) == TH_HTTP_1_0);
        TH_EXPECT(request.close == false);
        TH_EXPECT(th_find_header(&request, "Host") == NULL);
        TH_EXPECT(th_find_cookie(&request, "session") == NULL);
        TH_EXPECT(th_find_queryvar(&request, "a") == NULL);
        TH_EXPECT(th_find_formvar(&request, "b") == NULL);
        TH_EXPECT(th_find_pathvar(&request, "c") == NULL);
        TH_EXPECT(th_find_part(&request, "f") == NULL);
    }
    TH_TEST_CASE_END

    th_request_deinit(&request);
}
TH_TEST_END
