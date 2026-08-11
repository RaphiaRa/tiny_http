#ifndef TH_ROUTER_H
#define TH_ROUTER_H

#include <th.h>

#include "th_allocator.h"
#include "th_request.h"
#include "th_response.h"
#include "th_str.h"
#include "th_string.h"

typedef struct th_route_handler {
    th_handler handler;
    void* user_data;
} th_route_handler;

typedef struct th_capture {
    th_str key;
    th_str value;
} th_capture;

/** th_router_capture_cb
 * @brief Called per capture found while resolving a path. NULL = dry run.
 */
typedef void (*th_router_capture_cb)(void* userp, th_str key, th_str value);

typedef enum th_capture_type {
    TH_CAPTURE_TYPE_NONE = 0,
    TH_CAPTURE_TYPE_INT,
    TH_CAPTURE_TYPE_STRING,
    TH_CAPTURE_TYPE_PATH,
} th_capture_type;

typedef struct th_ws_route_handler {
    th_ws_handler handler;
    void* user_data;
} th_ws_route_handler;

typedef struct th_route_segment th_route_segment;
struct th_route_segment {
    th_capture_type type;
    th_string name;
    th_route_handler handler[TH_METHOD_MAX];
    th_ws_route_handler ws_handler;
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

/** th_router_would_handle
 *  Check if the router would handle the request, if
 * it was to be passed with the given method (and not the actual method in the request).
 */
TH_PRIVATE(bool)
th_router_would_handle(th_router* router, th_method method, th_request* request);

TH_PRIVATE(th_err)
th_router_add_route(th_router* router, th_method method, th_str route, th_handler handler, void* user_data);

TH_PRIVATE(th_err)
th_router_add_ws_route(th_router* router, th_str route, th_ws_handler handler, void* user_data);

/** th_router_find_ws_route
 * @brief Resolves path to a registered WS route (ignoring method). On a
 * match, sets handler and user_data and returns true.
 */
TH_PRIVATE(bool)
th_router_find_ws_route(th_router* router, th_str path, th_ws_handler* handler, void** user_data);

#endif
