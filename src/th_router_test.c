#include "th_router.h"
#include "th_test.h"

#include <string.h>

static const struct keyval {
    const char* key;
    const char* value;
}* required = NULL;
static size_t num_required = 0;
static bool success = true;

#define ROUTER_TEST_CASE_BEGIN(name)  \
    TH_TEST_CASE_BEGIN(router_##name) \
    {                                 \
        num_required = 0;

#define ROUTER_TEST_CASE_END \
    TH_TEST_CASE_END         \
    }

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

TH_TEST_BEGIN(router){
    ROUTER_TEST_CASE_BEGIN(router_init){
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
ROUTER_TEST_CASE_END
ROUTER_TEST_CASE_BEGIN(router_add_route)
{
    th_router router;
    th_router_init(&router, NULL);
    th_err err = th_router_add_route(&router, TH_METHOD_GET, TH_STRING("/test"), mock_handler, NULL);
    TH_EXPECT(err == TH_ERR_OK);
    th_router_deinit(&router);
}
ROUTER_TEST_CASE_END
ROUTER_TEST_CASE_BEGIN(router_handle)
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
ROUTER_TEST_CASE_END
ROUTER_TEST_CASE_BEGIN(router_handle_empty)
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
ROUTER_TEST_CASE_END
ROUTER_TEST_CASE_BEGIN(router_handle_path_capture)
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
        required = (const struct keyval[]){
            {.key = "path", .value = "abc"},
        };
        num_required = 1;
        TH_EXPECT(th_router_handle(&router, &request, &response) == TH_ERR_OK);
        TH_EXPECT(success);
        th_request_deinit(&request);
    }
    {
        th_request request = {0};
        th_request_init(&request, NULL, NULL);
        request.method = TH_METHOD_GET;
        th_heap_string_set(&request.uri_path, TH_STRING("/test/abc/def"));
        th_response response = {0};
        required = (const struct keyval[]){
            {.key = "path", .value = "abc/def"},
        };
        num_required = 1;
        TH_EXPECT(th_router_handle(&router, &request, &response) == TH_ERR_OK);
        TH_EXPECT(success);
        th_request_deinit(&request);
    }
    th_router_deinit(&router);
}
ROUTER_TEST_CASE_END
ROUTER_TEST_CASE_BEGIN(router_handle_capture_default)
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
        required = (const struct keyval[]){
            {.key = "first", .value = "abc"},
            {.key = "second", .value = "def"},
        };
        num_required = 2;
        TH_EXPECT(th_router_handle(&router, &request, &response) == TH_ERR_OK);
        TH_EXPECT(success);
        th_request_deinit(&request);
    }
    th_router_deinit(&router);
}
ROUTER_TEST_CASE_END
ROUTER_TEST_CASE_BEGIN(router_handle_capture_int)
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
        required = (const struct keyval[]){
            {.key = "id", .value = "123"},
        };
        num_required = 1;
        TH_EXPECT(th_router_handle(&router, &request, &response) == TH_ERR_OK);
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
ROUTER_TEST_CASE_END
ROUTER_TEST_CASE_BEGIN(router_handle_invalid_capture)
{
    th_router router;
    th_router_init(&router, NULL);
    TH_EXPECT(th_router_add_route(&router, TH_METHOD_GET, TH_STRING("/test/{invalid:arg}"), mock_handler, NULL) == TH_ERR_INVALID_ARG);
    TH_EXPECT(th_router_add_route(&router, TH_METHOD_GET, TH_STRING("/test/asdsad{invalid}"), mock_handler, NULL) == TH_ERR_INVALID_ARG);
    th_router_deinit(&router);
}
ROUTER_TEST_CASE_END
ROUTER_TEST_CASE_BEGIN(router_handle_multiple_simple_routes)
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
ROUTER_TEST_CASE_END
ROUTER_TEST_CASE_BEGIN(router_handle_multiple_complex_routes)
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
    for (size_t i = 0; i < sizeof(routes) / sizeof(routes[0]); i++) {
        TH_EXPECT(th_router_add_route(&router, TH_METHOD_GET, th_string_from_cstr(routes[i][0]), mock_handler, NULL) == TH_ERR_OK);
    }
    for (size_t i = 0; i < sizeof(routes) / sizeof(routes[0]); i++) {
        th_request request = {0};
        th_request_init(&request, NULL, NULL);
        request.method = TH_METHOD_GET;
        th_heap_string_set(&request.uri_path, th_string_from_cstr(routes[i][1]));
        th_response response = {0};
        struct keyval required_buf[10];
        required = required_buf;
        num_required = 0;
        // check captures
        for (size_t j = 2; routes[i][j] != NULL && routes[i][j + 1] != NULL; j += 2) {
            required_buf[(j - 2) / 2] = (struct keyval){.key = routes[i][j], .value = routes[i][j + 1]};
            num_required++;
        }
        TH_EXPECT(th_router_handle(&router, &request, &response) == TH_ERR_OK);
        TH_EXPECT(success);
        th_request_deinit(&request);
    }
    th_router_deinit(&router);
}
ROUTER_TEST_CASE_END
}
TH_TEST_END
