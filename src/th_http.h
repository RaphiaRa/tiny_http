#ifndef TH_HTTP_H
#define TH_HTTP_H

#include <th.h>

#include "th_config.h"
#include "th_conn.h"
#include "th_conn_tracker.h"
#include "th_fcache.h"
#include "th_request.h"
#include "th_request_parser.h"
#include "th_response.h"
#include "th_router.h"

typedef struct th_http th_http;

typedef enum th_http_state {
    TH_HTTP_STATE_READ_REQUEST,
    TH_HTTP_STATE_WRITE_RESPONSE,
} th_http_state;

typedef struct th_http_io_handler {
    th_io_handler base;
    th_http* http;
} th_http_io_handler;

struct th_http {
    const th_conn_tracker* tracker;
    th_http_io_handler io_handler;
    th_request_parser parser;
    th_request request;
    th_response response;
    th_buf_vec buf;
    th_conn* conn;
    th_router* router;
    th_fcache* fcache;
    th_allocator* allocator;
    size_t read_bytes;
    size_t parsed_bytes;

    // the current state of the http connection
    th_http_state state;

    // true if the connection should be closed
    bool close;
};

typedef struct th_http_upgrader {
    th_conn_upgrader base;
    const th_conn_tracker* tracker;
    th_router* router;
    th_fcache* fcache;
    th_allocator* allocator;
} th_http_upgrader;

TH_PRIVATE(void)
th_http_upgrader_init(th_http_upgrader* upgrader, const th_conn_tracker* tracker, th_router* router,
                      th_fcache* fcache, th_allocator* allocator);

#endif
