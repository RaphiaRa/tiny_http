#include "th_router.h"
#include "th_test.h"

#include <string.h>

static const struct keyval {
    const char* key;
    const char* value;
}* required = NULL;
static size_t num_required = 0;
static bool success = true;

static th_err
mock_handler(void* user_data, const th_request* req, th_response* resp)
{
    (void)user_data;
    (void)resp;
    for (size_t i = 0; i < num_required; i++) {
        const char* value = th_find_pathvar(req, required[i].key);
        if (!value || strcmp(value, required[i].value) != 0) {
            success = false;
            return TH_ERR_OK;
        }
    }
    success = true;
    return TH_ERR_OK;
}

TH_TEST_BEGIN(router)
{
    TH_TEST_CASE_BEGIN(router_init)
    {
        th_router router;
        th_router_init(&router, NULL);
        th_request request = {0};
        th_request_init(&request, NULL, NULL);
        request.method = TH_METHOD_GET;
        th_heap_string_set(&request.uri_path, TH_STRING("/test"));
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
        th_request_init(&request, NULL, NULL);
        request.method = TH_METHOD_GET;
        th_heap_string_set(&request.uri_path, TH_STRING("/test"));
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
            th_request_init(&request, NULL, NULL);
            request.method = TH_METHOD_GET;
            th_heap_string_set(&request.uri_path, TH_STRING("/"));
            th_response response = {0};
            TH_EXPECT(th_router_handle(&router, &request, &response) == TH_ERR_OK);
            th_request_deinit(&request);
        }
        {
            th_request request = {0};
            th_request_init(&request, NULL, NULL);
            request.method = TH_METHOD_GET;
            th_heap_string_set(&request.uri_path, TH_STRING("/test"));
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
            th_request_init(&request, NULL, NULL);
            request.method = TH_METHOD_GET;
            th_heap_string_set(&request.uri_path, TH_STRING("/test/abc"));
            th_response response = {0};
            TH_EXPECT(th_router_handle(&router, &request, &response) == TH_ERR_OK);
            required = (const struct keyval[]){
                {.key = "path", .value = "abc"},
            };
            TH_EXPECT(success);
            th_request_deinit(&request);
        }
        {
            th_request request = {0};
            th_request_init(&request, NULL, NULL);
            request.method = TH_METHOD_GET;
            th_heap_string_set(&request.uri_path, TH_STRING("/test/abc/def"));
            th_response response = {0};
            TH_EXPECT(th_router_handle(&router, &request, &response) == TH_ERR_OK);
            required = (const struct keyval[]){
                {.key = "path", .value = "abc/def"},
            };
            TH_EXPECT(success);
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
            th_request_init(&request, NULL, NULL);
            request.method = TH_METHOD_GET;
            th_heap_string_set(&request.uri_path, TH_STRING("/test/abc/test2/def"));
            th_response response = {0};
            TH_EXPECT(th_router_handle(&router, &request, &response) == TH_ERR_OK);
            required = (const struct keyval[]){
                {.key = "first", .value = "abc"},
                {.key = "second", .value = "def"},
            };
            TH_EXPECT(success);
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
            th_request_init(&request, NULL, NULL);
            request.method = TH_METHOD_GET;
            th_heap_string_set(&request.uri_path, TH_STRING("/test/123"));
            th_response response = {0};
            TH_EXPECT(th_router_handle(&router, &request, &response) == TH_ERR_OK);
            required = (const struct keyval[]){
                {.key = "id", .value = "123"},
            };
            TH_EXPECT(success);
            th_request_deinit(&request);
        }
        {
            th_request request = {0};
            th_request_init(&request, NULL, NULL);
            request.method = TH_METHOD_GET;
            th_heap_string_set(&request.uri_path, TH_STRING("/test/abc"));
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
    TH_TEST_CASE_BEGIN(router_handle_multiple_simple_routes)
    {
        th_router router;
        th_router_init(&router, NULL);
        const char* routes[] = {"/first", "/second", "/third", "/fourth", "/fifth"};
        for (size_t i = 0; i < sizeof(routes) / sizeof(routes[0]); i++) {
            TH_EXPECT(th_router_add_route(&router, TH_METHOD_GET, th_string_from_cstr(routes[i]), mock_handler, NULL) == TH_ERR_OK);
        }
        for (size_t i = 0; i < sizeof(routes) / sizeof(routes[0]); i++) {
            th_request request = {0};
            th_request_init(&request, NULL, NULL);
            request.method = TH_METHOD_GET;
            th_heap_string_set(&request.uri_path, th_string_from_cstr(routes[i]));
            th_response response = {0};
            TH_EXPECT(th_router_handle(&router, &request, &response) == TH_ERR_OK);
            th_request_deinit(&request);
        }
        th_router_deinit(&router);
    }
    TH_TEST_CASE_END
}
TH_TEST_END
