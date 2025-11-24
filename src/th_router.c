#include "th_router.h"
#include "th_allocator.h"
#include "th_log.h"
#include "th_request.h"
#include "th_string.h"
#include "th_url_decode.h"
#include "th_utility.h"

#include <assert.h>
#include <string.h>

#undef TH_LOG_TAG
#define TH_LOG_TAG "router"

TH_LOCAL(th_err)
th_route_init(th_route_segment* route, th_capture_type type, th_string segment, th_allocator* allocator)
{
    th_heap_string_init(&route->name, allocator);
    th_err err = TH_ERR_OK;
    if ((err = th_heap_string_set(&route->name, segment)) != TH_ERR_OK) {
        th_heap_string_deinit(&route->name);
        return err;
    }
    route->type = type;
    route->next = NULL;
    route->children = NULL;
    route->allocator = allocator;
    for (size_t i = 0; i < TH_METHOD_MAX; ++i)
        route->handler[i] = (th_route_handler){NULL, NULL};
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_route_create(th_route_segment** out, th_capture_type type, th_string token, th_allocator* allocator)
{
    th_route_segment* route = th_allocator_alloc(allocator, sizeof(th_route_segment));
    if (!route)
        return TH_ERR_BAD_ALLOC;
    th_err err = TH_ERR_OK;
    if ((err = th_route_init(route, type, token, allocator)) != TH_ERR_OK) {
        th_allocator_free(allocator, route);
        return err;
    }
    *out = route;
    return TH_ERR_OK;
}

TH_LOCAL(void)
th_route_destroy(th_route_segment* route);

TH_LOCAL(void)
th_route_deinit(th_route_segment* route)
{
    for (th_route_segment* child = route->children; child != NULL;) {
        th_route_segment* next = child->next;
        th_route_destroy(child);
        child = next;
    }
    th_heap_string_deinit(&route->name);
}

TH_LOCAL(void)
th_route_destroy(th_route_segment* route)
{
    th_route_deinit(route);
    th_allocator_free(route->allocator, route);
}

TH_PRIVATE(void)
th_router_init(th_router* router, th_allocator* allocator)
{
    router->routes = NULL;
    router->allocator = allocator;
    if (!router->allocator)
        router->allocator = th_default_allocator_get();
}

TH_PRIVATE(void)
th_router_deinit(th_router* router)
{
    for (th_route_segment* route = router->routes; route != NULL;) {
        th_route_segment* next = route->next;
        th_route_destroy(route);
        route = next;
    }
}

TH_LOCAL(th_err)
th_route_consume_trail(th_route_segment* route, th_request* request, th_string* trail, bool dry, bool* result)
{
    th_string route_name = th_heap_string_view(&route->name);
    th_heap_string decoded = {0};
    th_heap_string_init(&decoded, route->allocator);
    th_err err = TH_ERR_OK;
    if ((err = th_url_decode_string(th_string_substr(*trail, 0, th_string_find_first_of(*trail, 0, "/?")), &decoded, TH_URL_DECODE_TYPE_PATH))
        != TH_ERR_OK) {
        goto cleanup;
    }
    th_string segment = th_heap_string_view(&decoded);
    // if (th_string_empty(segment) && route->type != TH_CAPTURE_TYPE_NONE)
    //     return false;
    switch (route->type) {
    case TH_CAPTURE_TYPE_NONE:
        if (th_string_eq(route_name, segment)) {
            *trail = th_string_substr(*trail, segment.len + 1, th_string_npos);
            *result = true;
        }
        break;
    case TH_CAPTURE_TYPE_INT:
        if (th_string_is_uint(segment)) {
            if (!dry)
                (void)th_request_add_pathvar(request, route_name, segment);
            *trail = th_string_substr(*trail, segment.len + 1, th_string_npos);
            *result = true;
        }
        break;
    case TH_CAPTURE_TYPE_STRING:
        if (!dry)
            (void)th_request_add_pathvar(request, route_name, segment);
        *trail = th_string_substr(*trail, segment.len + 1, th_string_npos);
        *result = true;
        break;
    case TH_CAPTURE_TYPE_PATH:
        if (!dry)
            (void)th_request_add_pathvar(request, route_name, *trail);
        *trail = th_string_make(NULL, 0);
        *result = true;
        break;
    default:
        break;
    }
cleanup:
    th_heap_string_deinit(&decoded);
    return err;
}

TH_LOCAL(th_err)
th_router_do_handle(th_router* router, th_method method, th_request* request, th_response* response, bool dry)
{
    TH_LOG_DEBUG("Handling request %p: %s", request, th_heap_string_data(&request->uri_path));
    if (*th_heap_string_at(&request->uri_path, 0) != '/')
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    th_string trail = th_string_substr(th_heap_string_view(&request->uri_path), 1, th_string_npos);
    th_route_segment* route = router->routes;
    while (1) {
        th_err err = TH_ERR_OK;
        bool consumed = false;
        if (route == NULL) {
            break;
        } else if ((err = th_route_consume_trail(route, request, &trail, dry, &consumed)) != TH_ERR_OK
                   || consumed) {
            if (err != TH_ERR_OK)
                return err;
            if (th_string_empty(trail))
                break;
            route = route->children;
        } else {
            route = route->next;
        }
    }
    if (route == NULL) {
        return TH_ERR_HTTP(TH_CODE_NOT_FOUND);
    }
    th_route_handler handler = route->handler[method].handler ? route->handler[method] : route->handler[TH_METHOD_ANY];
    if (handler.handler == NULL) {
        return TH_ERR_HTTP(TH_CODE_METHOD_NOT_ALLOWED);
    }
    if (dry)
        return TH_ERR_OK;
    return handler.handler(handler.user_data, request, response);
}

TH_PRIVATE(th_err)
th_router_handle(th_router* router, th_request* request, th_response* response)
{
    return th_router_do_handle(router, request->method, request, response, false);
}

TH_PRIVATE(bool)
th_router_would_handle(th_router* router, th_method method, th_request* request)
{
    return th_router_do_handle(router, method, request, NULL, true) == TH_ERR_OK;
}

// abc < {int} < {string} < {path}
TH_LOCAL(bool)
th_route_lower(th_route_segment* lh, th_route_segment* rh)
{
    return lh->type < rh->type;
}

TH_LOCAL(void)
th_route_insert_sorted(th_route_segment** list, th_route_segment* route)
{
    while (*list != NULL && th_route_lower(*list, route))
        list = &(*list)->next;
    th_route_segment* temp = *list;
    *list = route;
    route->next = temp;
}

TH_LOCAL(th_err)
th_route_parse_trail(th_string* trail, th_string* name, th_capture_type* type)
{
    th_string segment = th_string_substr(*trail, 0, th_string_find_first_of(*trail, 0, "/"));
    size_t open_curly = th_string_find_first(segment, 0, '{');
    size_t close_curly = th_string_find_first(segment, 0, '}');
    if (segment.len > 2 && open_curly == 0 && close_curly == segment.len - 1) {
        th_string capture = th_string_substr(segment, 1, segment.len - 2);
        size_t sep = th_string_find_first(capture, 0, ':');
        if (sep == th_string_npos) {
            *name = capture;
            *type = TH_CAPTURE_TYPE_STRING;
        } else {
            th_string type_str = th_string_substr(capture, 0, sep);
            if (th_string_eq(type_str, TH_STRING("int"))) {
                *name = th_string_substr(capture, sep + 1, th_string_npos);
                *type = TH_CAPTURE_TYPE_INT;
            } else if (th_string_eq(type_str, TH_STRING("path"))) {
                *name = th_string_substr(capture, sep + 1, th_string_npos);
                *type = TH_CAPTURE_TYPE_PATH;
            } else {
                return TH_ERR_INVALID_ARG;
            }
        }
    } else if (open_curly == th_string_npos && close_curly == th_string_npos) {
        *name = segment;
        *type = TH_CAPTURE_TYPE_NONE;
    } else {
        return TH_ERR_INVALID_ARG;
    }
    // Consume segment
    *trail = th_string_substr(*trail, segment.len + 1, th_string_npos);
    return TH_ERR_OK;
}

TH_PRIVATE(th_err)
th_router_add_route(th_router* router, th_method method, th_string path, th_handler handler, void* user_data)
{
    if (th_string_empty(path) || path.ptr[0] != '/')
        return TH_ERR_INVALID_ARG;
    th_string trail = th_string_substr(path, 1, th_string_npos);
    th_route_segment** list = &router->routes;
    th_route_segment* route = *list;

    // find a matching route
    bool last = false;
    while (!last) {
        th_string name = {0};
        th_capture_type type = TH_CAPTURE_TYPE_NONE;
        th_err err = TH_ERR_OK;
        if ((err = th_route_parse_trail(&trail, &name, &type)) != TH_ERR_OK)
            return err;
        last = th_string_empty(trail);
        if (type == TH_CAPTURE_TYPE_PATH && !last)
            return TH_ERR_INVALID_ARG;
        while (1) {
            if (route == NULL) {
                if ((err = th_route_create(&route, type, name, router->allocator)) != TH_ERR_OK)
                    return err;
                th_route_insert_sorted(list, route);
                route = *list; // restart
            }
            if ((type == TH_CAPTURE_TYPE_NONE
                 && th_string_eq(th_heap_string_view(&route->name), name))
                || (type != TH_CAPTURE_TYPE_NONE && type == route->type)) {
                if (last)
                    break;
                list = &route->children;
                route = *list;
                break;
            } else {
                route = route->next;
            }
        }
    }

    if (route->handler[TH_METHOD_ANY].handler != NULL
        || route->handler[method].handler != NULL)
        return TH_ERR_INVALID_ARG; // Route already exists
    route->handler[method].handler = handler;
    route->handler[method].user_data = user_data;
    return TH_ERR_OK;
}
