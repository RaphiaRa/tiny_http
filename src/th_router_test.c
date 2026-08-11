#include "th_router.h"
#include "th_test.h"
#include "th_utility.h"

#include <string.h>

#define ROUTER_TEST_MAX_CAPTURES 4

struct keyval {
    const char* key;
    const char* value;
};

struct route_expectations {
    const struct keyval* pairs;
    size_t count;
};

static th_err
expect_pathvars_handler(void* user_data, const th_request* req, th_response* resp)
{
    (void)resp;
    const struct route_expectations* expected = user_data;
    if (!expected)
        return TH_ERR_OK;
    for (size_t i = 0; i < expected->count; i++) {
        const char* value = th_find_pathvar(req, expected->pairs[i].key);
        if (!value || strcmp(value, expected->pairs[i].value) != 0) {
            return TH_ERR_INVALID_ARG;
        }
    }
    return TH_ERR_OK;
}

static th_err
noop_ws_handler(void* userp, th_ws* ws, th_ws_event ev, th_buffer data)
{
    (void)userp;
    (void)ws;
    (void)ev;
    (void)data;
    return TH_ERR_OK;
}

TH_TEST_BEGIN(router)
{
    TH_TEST_CASE_BEGIN(router_init)
    {
        th_router router;
        th_router_init(&router, NULL);
        th_request request = {0};
        th_request_init(&request, NULL);
        request.method = TH_METHOD_GET;
        th_string_set(&request.uri_path, TH_STR("/test"));
        th_response response = {0};
        TH_EXPECT(th_router_handle(&router, &request, &response) == TH_ERR_HTTP(TH_CODE_NOT_FOUND));
        th_router_deinit(&router);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(router_add_route)
    {
        th_router router;
        th_router_init(&router, NULL);
        th_err err = th_router_add_route(&router, TH_METHOD_GET, TH_STR("/test"), expect_pathvars_handler, NULL);
        TH_EXPECT(err == TH_ERR_OK);
        th_router_deinit(&router);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(router_handle)
    {
        th_router router;
        th_router_init(&router, NULL);
        th_err err = th_router_add_route(&router, TH_METHOD_GET, TH_STR("/test"), expect_pathvars_handler, NULL);
        TH_EXPECT(err == TH_ERR_OK);
        th_request request = {0};
        th_request_init(&request, NULL);
        request.method = TH_METHOD_GET;
        th_string_set(&request.uri_path, TH_STR("/test"));
        th_response response = {0};
        TH_EXPECT(th_router_handle(&router, &request, &response) == TH_ERR_OK);
        th_router_deinit(&router);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(router_handle_empty)
    {
        th_router router;
        th_router_init(&router, NULL);
        TH_EXPECT(th_router_add_route(&router, TH_METHOD_GET, TH_STR("/"), expect_pathvars_handler, NULL) == TH_ERR_OK);
        {
            th_request request = {0};
            th_request_init(&request, NULL);
            request.method = TH_METHOD_GET;
            th_string_set(&request.uri_path, TH_STR("/"));
            th_response response = {0};
            TH_EXPECT(th_router_handle(&router, &request, &response) == TH_ERR_OK);
            th_request_deinit(&request);
        }
        {
            th_request request = {0};
            th_request_init(&request, NULL);
            request.method = TH_METHOD_GET;
            th_string_set(&request.uri_path, TH_STR("/test"));
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
        {
            const struct keyval pairs[] = {
                {.key = "path", .value = "abc"},
            };
            struct route_expectations expected = {.pairs = pairs, .count = 1};
            th_err err = th_router_add_route(&router, TH_METHOD_GET, TH_STR("/test/{path:path}"), expect_pathvars_handler, &expected);
            TH_EXPECT(err == TH_ERR_OK);

            th_request request = {0};
            th_request_init(&request, NULL);
            request.method = TH_METHOD_GET;
            th_string_set(&request.uri_path, TH_STR("/test/abc"));
            th_response response = {0};
            TH_EXPECT(th_router_handle(&router, &request, &response) == TH_ERR_OK);
            th_request_deinit(&request);
        }
        th_router_deinit(&router);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(router_handle_path_capture_multi_segment)
    {
        th_router router;
        th_router_init(&router, NULL);
        {
            const struct keyval pairs[] = {
                {.key = "path", .value = "abc/def"},
            };
            struct route_expectations expected = {.pairs = pairs, .count = 1};
            th_err err = th_router_add_route(&router, TH_METHOD_GET, TH_STR("/test/{path:path}"), expect_pathvars_handler, &expected);
            TH_EXPECT(err == TH_ERR_OK);

            th_request request = {0};
            th_request_init(&request, NULL);
            request.method = TH_METHOD_GET;
            th_string_set(&request.uri_path, TH_STR("/test/abc/def"));
            th_response response = {0};
            TH_EXPECT(th_router_handle(&router, &request, &response) == TH_ERR_OK);
            th_request_deinit(&request);
        }
        th_router_deinit(&router);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(router_handle_capture_default)
    {
        th_router router;
        th_router_init(&router, NULL);
        const struct keyval pairs[] = {
            {.key = "first", .value = "abc"},
            {.key = "second", .value = "def"},
        };
        struct route_expectations expected = {.pairs = pairs, .count = 2};
        th_err err = th_router_add_route(
            &router, TH_METHOD_GET, TH_STR("/test/{first}/test2/{second}"), expect_pathvars_handler, &expected);
        TH_EXPECT(err == TH_ERR_OK);
        {
            th_request request = {0};
            th_request_init(&request, NULL);
            request.method = TH_METHOD_GET;
            th_string_set(&request.uri_path, TH_STR("/test/abc/test2/def"));
            th_response response = {0};
            TH_EXPECT(th_router_handle(&router, &request, &response) == TH_ERR_OK);
            th_request_deinit(&request);
        }
        th_router_deinit(&router);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(router_handle_capture_int)
    {
        th_router router;
        th_router_init(&router, NULL);
        const struct keyval pairs[] = {
            {.key = "id", .value = "123"},
        };
        struct route_expectations expected = {.pairs = pairs, .count = 1};
        th_err err = th_router_add_route(&router, TH_METHOD_GET, TH_STR("/test/{int:id}"), expect_pathvars_handler, &expected);
        TH_EXPECT(err == TH_ERR_OK);
        {
            th_request request = {0};
            th_request_init(&request, NULL);
            request.method = TH_METHOD_GET;
            th_string_set(&request.uri_path, TH_STR("/test/123"));
            th_response response = {0};
            TH_EXPECT(th_router_handle(&router, &request, &response) == TH_ERR_OK);
            th_request_deinit(&request);
        }
        {
            th_request request = {0};
            th_request_init(&request, NULL);
            request.method = TH_METHOD_GET;
            th_string_set(&request.uri_path, TH_STR("/test/abc"));
            th_response response = {0};
            TH_EXPECT(th_router_handle(&router, &request, &response) == TH_ERR_HTTP(TH_CODE_NOT_FOUND));
            th_request_deinit(&request);
        }
        th_router_deinit(&router);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(router_handle_url_encoded_segment)
    {
        th_router router;
        th_router_init(&router, NULL);
        th_err err = th_router_add_route(&router, TH_METHOD_GET, TH_STR("/a b"), expect_pathvars_handler, NULL);
        TH_EXPECT(err == TH_ERR_OK);
        {
            th_request request = {0};
            th_request_init(&request, NULL);
            request.method = TH_METHOD_GET;
            th_string_set(&request.uri_path, TH_STR("/a%20b"));
            th_response response = {0};
            TH_EXPECT(th_router_handle(&router, &request, &response) == TH_ERR_OK);
            th_request_deinit(&request);
        }
        th_router_deinit(&router);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(router_handle_capture_url_encoded)
    {
        th_router router;
        th_router_init(&router, NULL);
        const struct keyval pairs[] = {
            {.key = "name", .value = "a b"},
        };
        struct route_expectations expected = {.pairs = pairs, .count = 1};
        th_err err = th_router_add_route(&router, TH_METHOD_GET, TH_STR("/test/{name}"), expect_pathvars_handler, &expected);
        TH_EXPECT(err == TH_ERR_OK);
        {
            th_request request = {0};
            th_request_init(&request, NULL);
            request.method = TH_METHOD_GET;
            th_string_set(&request.uri_path, TH_STR("/test/a%20b"));
            th_response response = {0};
            TH_EXPECT(th_router_handle(&router, &request, &response) == TH_ERR_OK);
            th_request_deinit(&request);
        }
        th_router_deinit(&router);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(router_handle_url_encoded_segment_not_last)
    {
        th_router router;
        th_router_init(&router, NULL);
        th_err err = th_router_add_route(&router, TH_METHOD_GET, TH_STR("/a b/test"), expect_pathvars_handler, NULL);
        TH_EXPECT(err == TH_ERR_OK);
        {
            th_request request = {0};
            th_request_init(&request, NULL);
            request.method = TH_METHOD_GET;
            th_string_set(&request.uri_path, TH_STR("/a%20b/test"));
            th_response response = {0};
            TH_EXPECT(th_router_handle(&router, &request, &response) == TH_ERR_OK);
            th_request_deinit(&request);
        }
        th_router_deinit(&router);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(router_handle_invalid_capture)
    {
        th_router router;
        th_router_init(&router, NULL);
        TH_EXPECT(
            th_router_add_route(&router, TH_METHOD_GET, TH_STR("/test/{invalid:arg}"), expect_pathvars_handler, NULL)
            == TH_ERR_INVALID_ARG);
        TH_EXPECT(
            th_router_add_route(&router, TH_METHOD_GET, TH_STR("/test/asdsad{invalid}"), expect_pathvars_handler, NULL)
            == TH_ERR_INVALID_ARG);
        th_router_deinit(&router);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(router_handle_multiple_simple_routes)
    {
        th_router router;
        th_router_init(&router, NULL);
        const char* routes[] = {"/first", "/second", "/third", "/fourth", "/fifth"};
        for (size_t i = 0; i < sizeof(routes) / sizeof(routes[0]); i++) {
            TH_EXPECT(
                th_router_add_route(&router, TH_METHOD_GET, th_str_from_cstr(routes[i]), expect_pathvars_handler, NULL)
                == TH_ERR_OK);
        }
        for (size_t i = 0; i < sizeof(routes) / sizeof(routes[0]); i++) {
            th_request request = {0};
            th_request_init(&request, NULL);
            request.method = TH_METHOD_GET;
            th_string_set(&request.uri_path, th_str_from_cstr(routes[i]));
            th_response response = {0};
            TH_EXPECT(th_router_handle(&router, &request, &response) == TH_ERR_OK);
            th_request_deinit(&request);
        }
        th_router_deinit(&router);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(router_handle_multiple_complex_routes)
    {
        th_router router;
        th_router_init(&router, NULL);
        // route, test, expected captures
        const char* routes[][10] = {
            {"/user/{int:id}/profile", "/user/42/profile", "id", "42", NULL},
            {"/product/{name}/details", "/product/widget/details", "name", "widget", NULL},
            {"/order/{int:order_id}/item/{item_name}", "/order/1001/item/gadget", "order_id", "1001", "item_name", "gadget", NULL},
            {"/category/{name}/page/{int:page_num}", "/category/electronics/page/2", "name", "electronics", "page_num", "2", NULL},
            {"/blog/{int:year}/{int:month}/{slug}", "/blog/2023/06/my-post", "year", "2023", "month", "06", "slug", "my-post", NULL},
            {"/files/{path:path}", "/files/documents/reports/2024", "path", "documents/reports/2024", NULL},
            {"/search/{query}/page/{int:page}", "/search/laptops/page/3", "query", "laptops", "page", "3", NULL},
            {"/profile/{username}/settings", "/profile/johndoe/settings", "username", "johndoe", NULL},
            {"/download/{int:file_id}/{filename}", "/download/555/manual.pdf", "file_id", "555", "filename", "manual.pdf", NULL},
            {"/event/{int:event_id}/attendee/{int:attendee_id}", "/event/200/attendee/1500", "event_id", "200", "attendee_id", "1500", NULL},
            {"/news/{category}/article/{slug}", "/news/technology/article/new-gadget-release", "category", "technology", "slug", "new-gadget-release", NULL},
            {"/gallery/{path:path}", "/gallery/2024/vacation/photos", "path", "2024/vacation/photos", NULL},
            {"/forum/{int:forum_id}/thread/{int:thread_id}", "/forum/10/thread/250", "forum_id", "10", "thread_id", "250", NULL},
            {"/video/{title}/watch", "/video/cool-video/watch", "title", "cool-video", NULL},
            {"/audio/{int:track_id}/play", "/audio/300/play", "track_id", "300", NULL},
            {"/document/{path:path}", "/document/work/reports/annual", "path", "work/reports/annual", NULL},
            {"/course/{int:course_id}/lesson/{int:lesson_id}", "/course/101/lesson/5", "course_id", "101", "lesson_id", "5", NULL},
            {"/recipe/{name}/details", "/recipe/chocolate-cake/details", "name", "chocolate-cake", NULL},
            {"/profile/{username}/photos", "/profile/alice/photos", "username", "alice", NULL},
            {"/project/{int:project_id}/task/{int:task_id}", "/project/77/task/300", "project_id", "77", "task_id", "300", NULL},
            {"/article/{slug}/comments", "/article/interesting-article/comments", "slug", "interesting-article", NULL},
            {"/shop/{category}/item/{int:item_id}", "/shop/electronics/item/999", "category", "electronics", "item_id", "999", NULL},
            {"/profile/{int:id}/photos", "/profile/123/photos", "id", "123", NULL},
            {"/blog/{slug}/edit", "/blog/my-first-post/edit", "slug", "my-first-post", NULL},
            {"/event/{int:event_id}/details", "/event/456/details", "event_id", "456", NULL},
            {"/store/{int:store_id}/product/{int:product_id}", "/store/12/product/34", "store_id", "12", "product_id", "34", NULL},
            {"/user/{username}/dashboard", "/user/bob/dashboard", "username", "bob", NULL},
            {"/order/{int:order_id}/status", "/order/789/status", "order_id", "789", NULL},
            {"/ticket/{int:ticket_id}/reply", "/ticket/555/reply", "ticket_id", "555", NULL},
            {"/message/{int:message_id}/read", "/message/888/read", "message_id", "888", NULL},
            {"/notification/{int:notification_id}/view", "/notification/777/view", "notification_id", "777", NULL},
            {"/comment/{int:comment_id}/like", "/comment/666/like", "comment_id", "666", NULL},
            {"/profile/{username}/followers", "/profile/charlie/followers", "username", "charlie", NULL},
            {"/profile/{username}/following", "/profile/dave/following", "username", "dave", NULL},
            {"/album/{int:album_id}/photos", "/album/321/photos", "album_id", "321", NULL},
        };
        size_t num_routes = sizeof(routes) / sizeof(routes[0]);
        struct keyval pairs[TH_ARRAY_SIZE(routes)][ROUTER_TEST_MAX_CAPTURES];
        struct route_expectations expected[TH_ARRAY_SIZE(routes)];
        for (size_t i = 0; i < num_routes; i++) {
            size_t count = 0;
            for (size_t j = 2; routes[i][j] != NULL && routes[i][j + 1] != NULL; j += 2) {
                pairs[i][count] = (struct keyval){.key = routes[i][j], .value = routes[i][j + 1]};
                count++;
            }
            expected[i] = (struct route_expectations){.pairs = pairs[i], .count = count};
            TH_EXPECT(
                th_router_add_route(&router, TH_METHOD_GET, th_str_from_cstr(routes[i][0]), expect_pathvars_handler, &expected[i])
                == TH_ERR_OK);
        }
        for (size_t i = 0; i < num_routes; i++) {
            th_request request = {0};
            th_request_init(&request, NULL);
            request.method = TH_METHOD_GET;
            th_string_set(&request.uri_path, th_str_from_cstr(routes[i][1]));
            th_response response = {0};
            TH_EXPECT(th_router_handle(&router, &request, &response) == TH_ERR_OK);
            th_request_deinit(&request);
        }
        th_router_deinit(&router);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(router_find_ws_route)
    {
        th_router router;
        th_router_init(&router, NULL);
        int userp = 0;
        TH_EXPECT(th_router_add_ws_route(&router, TH_STR("/ws"), noop_ws_handler, &userp) == TH_ERR_OK);
        th_ws_handler handler = NULL;
        void* user_data = NULL;
        TH_EXPECT(th_router_find_ws_route(&router, TH_STR("/ws"), &handler, &user_data));
        TH_EXPECT(handler == noop_ws_handler);
        TH_EXPECT(user_data == &userp);
        th_router_deinit(&router);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(router_find_ws_route_not_registered)
    {
        th_router router;
        th_router_init(&router, NULL);
        TH_EXPECT(th_router_add_route(&router, TH_METHOD_GET, TH_STR("/plain"), expect_pathvars_handler, NULL) == TH_ERR_OK);
        th_ws_handler handler = NULL;
        void* user_data = NULL;
        TH_EXPECT(!th_router_find_ws_route(&router, TH_STR("/plain"), &handler, &user_data));
        th_router_deinit(&router);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(router_ws_route_and_http_route_coexist)
    {
        th_router router;
        th_router_init(&router, NULL);
        TH_EXPECT(th_router_add_route(&router, TH_METHOD_GET, TH_STR("/chat"), expect_pathvars_handler, NULL) == TH_ERR_OK);
        TH_EXPECT(th_router_add_ws_route(&router, TH_STR("/chat"), noop_ws_handler, NULL) == TH_ERR_OK);
        th_request request = {0};
        th_request_init(&request, NULL);
        request.method = TH_METHOD_GET;
        th_string_set(&request.uri_path, TH_STR("/chat"));
        th_response response = {0};
        TH_EXPECT(th_router_handle(&router, &request, &response) == TH_ERR_OK);
        th_ws_handler handler = NULL;
        void* user_data = NULL;
        TH_EXPECT(th_router_find_ws_route(&router, th_string_view(&request.uri_path), &handler, &user_data));
        TH_EXPECT(handler == noop_ws_handler);
        th_request_deinit(&request);
        th_router_deinit(&router);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(router_add_ws_route_twice_fails)
    {
        th_router router;
        th_router_init(&router, NULL);
        TH_EXPECT(th_router_add_ws_route(&router, TH_STR("/ws"), noop_ws_handler, NULL) == TH_ERR_OK);
        TH_EXPECT(th_router_add_ws_route(&router, TH_STR("/ws"), noop_ws_handler, NULL) == TH_ERR_INVALID_ARG);
        th_router_deinit(&router);
    }
    TH_TEST_CASE_END
}
TH_TEST_END
