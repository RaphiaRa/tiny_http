#include "th_bench.h"
#include "th_request.h"
#include "th_response.h"
#include "th_router.h"

static th_err
router_bench_handler(void* user_data, const th_request* req, th_response* resp)
{
    (void)user_data;
    (void)req;
    (void)resp;
    return TH_ERR_OK;
}

TH_BENCH_BEGIN(router)
{
    TH_BENCH_CASE_BEGIN(handle_literal_route, 100000)
    {
        th_router router;
        th_router_init(&router, NULL);
        th_router_add_route(&router, TH_METHOD_GET, TH_STR("/test"), router_bench_handler, NULL);

        th_request request = {0};
        th_request_init(&request, NULL, NULL, NULL);
        request.method = TH_METHOD_GET;
        th_string_set(&request.uri_path, TH_STR("/test"));
        th_response response = {0};

        TH_BENCH_RUN_BEGIN
        {
            th_router_handle(&router, &request, &response);
        }
        TH_BENCH_RUN_END

        th_request_deinit(&request);
        th_router_deinit(&router);
    }
    TH_BENCH_CASE_END

    TH_BENCH_CASE_BEGIN(handle_captured_route, 100000)
    {
        th_router router;
        th_router_init(&router, NULL);
        th_router_add_route(
            &router, TH_METHOD_GET, TH_STR("/user/{int:id}/profile/{name}"), router_bench_handler, NULL);

        th_request request = {0};
        th_request_init(&request, NULL, NULL, NULL);
        request.method = TH_METHOD_GET;
        th_string_set(&request.uri_path, TH_STR("/user/42/profile/edit"));
        th_response response = {0};

        TH_BENCH_RUN_BEGIN
        {
            th_router_handle(&router, &request, &response);
        }
        TH_BENCH_RUN_END

        th_request_deinit(&request);
        th_router_deinit(&router);
    }
    TH_BENCH_CASE_END

    TH_BENCH_CASE_BEGIN(handle_with_many_sibling_routes, 100000)
    {
        th_router router;
        th_router_init(&router, NULL);
        const char* routes[] = {
            "/first",  "/second", "/third",  "/fourth", "/fifth",  "/sixth",   "/seventh", "/eighth",
            "/ninth",  "/tenth",  "/alpha",  "/bravo",  "/charlie", "/delta",  "/echo",    "/foxtrot",
            "/golf",   "/hotel",  "/india",  "/juliet", "/kilo",   "/lima",    "/mike",    "/november",
            "/oscar",  "/papa",   "/quebec", "/romeo",  "/sierra", "/tango",   "/uniform", "/victor",
        };
        for (size_t i = 0; i < sizeof(routes) / sizeof(routes[0]); i++) {
            th_router_add_route(&router, TH_METHOD_GET, th_str_from_cstr(routes[i]), router_bench_handler, NULL);
        }

        th_request request = {0};
        th_request_init(&request, NULL, NULL, NULL);
        request.method = TH_METHOD_GET;
        th_string_set(&request.uri_path, TH_STR("/victor"));
        th_response response = {0};

        TH_BENCH_RUN_BEGIN
        {
            th_router_handle(&router, &request, &response);
        }
        TH_BENCH_RUN_END

        th_request_deinit(&request);
        th_router_deinit(&router);
    }
    TH_BENCH_CASE_END
}
TH_BENCH_END
