#include "th_router.h"
#include "th_test.h"

#include <string.h>

static th_err
mock_handler(void* user_data, const th_request* req, th_response* resp)
{
    (void)user_data;
    (void)req;
    (void)resp;
    return TH_ERR_OK;
}

TH_TEST_BEGIN(router)
{
    TH_TEST_CASE_BEGIN(router_init)
    {
        th_router router;
        th_router_init(&router, NULL);
        th_request request = {0};
        request.method = TH_METHOD_GET;
        request.uri_path = "/test";
        th_response response = {0};
        TH_EXPECT(th_router_handle(&router, &request, &response) == TH_ERR_HTTP(TH_CODE_NOT_FOUND));
        th_router_deinit(&router);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(router_add_route)
    {
        th_router router;
        th_router_init(&router, NULL);
        th_err err = th_router_add_route(&router, TH_METHOD_GET, TH_STRING("/test"), mock_handler, NULL);
        TH_EXPECT(err == TH_ERR_OK);
        th_router_deinit(&router);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(router_handle)
    {
        th_router router;
        th_router_init(&router, NULL);
        th_err err = th_router_add_route(&router, TH_METHOD_GET, TH_STRING("/test"), mock_handler, NULL);
        TH_EXPECT(err == TH_ERR_OK);
        th_request request = {0};
        request.method = TH_METHOD_GET;
        request.uri_path = "/test";
        th_response response = {0};
        TH_EXPECT(th_router_handle(&router, &request, &response) == TH_ERR_OK);
        th_router_deinit(&router);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(router_handle_empty)
    {
        th_router router;
        th_router_init(&router, NULL);
        TH_EXPECT(th_router_add_route(&router, TH_METHOD_GET, TH_STRING("/"), mock_handler, NULL) == TH_ERR_OK);
        {
            th_request request = {0};
            th_request_init(&request, NULL);
            request.method = TH_METHOD_GET;
            request.uri_path = "/";
            th_response response = {0};
            TH_EXPECT(th_router_handle(&router, &request, &response) == TH_ERR_OK);
            th_request_deinit(&request);
        }
        {
            th_request request = {0};
            th_request_init(&request, NULL);
            request.method = TH_METHOD_GET;
            request.uri_path = "/test";
            th_response response = {0};
            TH_EXPECT(th_router_handle(&router, &request, &response) == TH_ERR_HTTP(TH_CODE_NOT_FOUND));
            th_request_deinit(&request);
        }
        th_router_deinit(&router);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(router_handle_path_capture)
    {
        th_router router;
        th_router_init(&router, NULL);
        th_err err = th_router_add_route(&router, TH_METHOD_GET, TH_STRING("/test/{path:path}"), mock_handler, NULL);
        TH_EXPECT(err == TH_ERR_OK);
        {
            th_request request = {0};
            th_request_init(&request, NULL);
            request.method = TH_METHOD_GET;
            request.uri_path = "/test/abc";
            th_response response = {0};
            TH_EXPECT(th_router_handle(&router, &request, &response) == TH_ERR_OK);
            const char* param = th_try_get_path_param(&request, "path");
            TH_EXPECT(param && strcmp(param, "abc") == 0);
            th_request_deinit(&request);
        }
        {
            th_request request = {0};
            th_request_init(&request, NULL);
            request.method = TH_METHOD_GET;
            request.uri_path = "/test/abc/def";
            th_response response = {0};
            TH_EXPECT(th_router_handle(&router, &request, &response) == TH_ERR_OK);
            const char* param = th_try_get_path_param(&request, "path");
            TH_EXPECT(param && strcmp(param, "abc/def") == 0);
            th_request_deinit(&request);
        }
        th_router_deinit(&router);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(router_handle_capture_default)
    {
        th_router router;
        th_router_init(&router, NULL);
        th_err err = th_router_add_route(&router, TH_METHOD_GET, TH_STRING("/test/{first}/test2/{second}"), mock_handler, NULL);
        TH_EXPECT(err == TH_ERR_OK);
        {
            th_request request = {0};
            th_request_init(&request, NULL);
            request.method = TH_METHOD_GET;
            request.uri_path = "/test/abc/test2/def";
            th_response response = {0};
            TH_EXPECT(th_router_handle(&router, &request, &response) == TH_ERR_OK);
            const char* param = th_try_get_path_param(&request, "first");
            TH_EXPECT(param && strcmp(param, "abc") == 0);
            param = th_try_get_path_param(&request, "second");
            TH_EXPECT(param && strcmp(param, "def") == 0);
            th_request_deinit(&request);
        }
        th_router_deinit(&router);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(router_handle_capture_int)
    {
        th_router router;
        th_router_init(&router, NULL);
        th_err err = th_router_add_route(&router, TH_METHOD_GET, TH_STRING("/test/{int:id}"), mock_handler, NULL);
        TH_EXPECT(err == TH_ERR_OK);
        {
            th_request request = {0};
            th_request_init(&request, NULL);
            request.method = TH_METHOD_GET;
            request.uri_path = "/test/123";
            th_response response = {0};
            TH_EXPECT(th_router_handle(&router, &request, &response) == TH_ERR_OK);
            const char* id = th_try_get_path_param(&request, "id");
            TH_EXPECT(strncmp(id, "123", 3) == 0);
            th_request_deinit(&request);
        }
        {
            th_request request = {0};
            th_request_init(&request, NULL);
            request.method = TH_METHOD_GET;
            request.uri_path = "/test/abc";
            th_response response = {0};
            TH_EXPECT(th_router_handle(&router, &request, &response) == TH_ERR_HTTP(TH_CODE_NOT_FOUND));
            th_request_deinit(&request);
        }
        th_router_deinit(&router);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(router_handle_invalid_capture)
    {
        th_router router;
        th_router_init(&router, NULL);
        TH_EXPECT(th_router_add_route(&router, TH_METHOD_GET, TH_STRING("/test/{invalid:arg}"), mock_handler, NULL) == TH_ERR_INVALID_ARG);
        TH_EXPECT(th_router_add_route(&router, TH_METHOD_GET, TH_STRING("/test/asdsad{invalid}"), mock_handler, NULL) == TH_ERR_INVALID_ARG);
        th_router_deinit(&router);
    }
    TH_TEST_CASE_END
}
TH_TEST_END
