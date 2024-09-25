#ifndef TH_ROUTER_H
#define TH_ROUTER_H

#include <th.h>

#include "th_allocator.h"
#include "th_heap_string.h"
#include "th_request.h"
#include "th_response.h"
#include "th_string.h"

typedef struct th_route_handler {
    th_handler handler;
    void* user_data;
} th_route_handler;

typedef struct th_capture {
    th_string key;
    th_string value;
} th_capture;

typedef enum th_capture_type {
    TH_CAPTURE_TYPE_NONE = 0,
    TH_CAPTURE_TYPE_INT,
    TH_CAPTURE_TYPE_STRING,
    TH_CAPTURE_TYPE_PATH,
} th_capture_type;

typedef struct th_route_segment th_route_segment;
struct th_route_segment {
    th_capture_type type;
    th_heap_string name;
    th_route_handler handler[TH_METHOD_MAX];
    th_route_segment* next;
    th_route_segment* children;
    th_allocator* allocator;
};

typedef struct th_router {
    th_route_segment* routes;
    th_allocator* allocator;
} th_router;

TH_PRIVATE(void)
th_router_init(th_router* router, th_allocator* allocator);

TH_PRIVATE(void)
th_router_deinit(th_router* router);

TH_PRIVATE(th_err)
th_router_handle(th_router* router, th_request* request, th_response* response);

TH_PRIVATE(th_err)
th_router_add_route(th_router* router, th_method method, th_string route, th_handler handler, void* user_data);

#endif
