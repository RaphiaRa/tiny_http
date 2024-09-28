#ifndef TH_REQUEST_H
#define TH_REQUEST_H

#include <th.h>

#include "th_config.h"
#include "th_hashmap.h"
#include "th_heap_string.h"
#include "th_method.h"
#include "th_socket.h"

#define TH_REQUEST_MAP_ARENA_LEN 512
#define TH_REQUEST_STRING_ARENA_LEN 512
#define TH_REQUEST_VEC_ARENA_LEN 1024

struct th_request {
    th_allocator* allocator;
    const char* uri_path;
    const char* uri_query;
    void* map_arena;
    void* vec_arena;
    void* string_arena;
    th_arena_allocator map_allocator;
    th_arena_allocator vec_allocator;
    th_arena_allocator string_allocator;
    th_cstr_map cookies;
    th_cstr_map headers;
    th_cstr_map query_params;
    th_cstr_map body_params;
    th_cstr_map path_params;
    /** heap_strings
     * This vector is used to store heap allocated strings that are used in the request.
     * It's used to ensure that all memory is deallocated when the request is destroyed.
     */
    th_heap_string_vec heap_strings;
    th_buf_vec buffer;
    /* content_len as specified in the Content-Length header */
    size_t content_len;
    size_t data_len;
    size_t content_buf_len;
    size_t content_buf_pos;
    char* content_buf;
    th_method_internal method_internal;
    th_method method;
    int minor_version;
    bool close;
    bool parse_body_params;
};

TH_PRIVATE(void)
th_request_init(th_request* request, th_allocator* allocator);

TH_PRIVATE(void)
th_request_deinit(th_request* request);

TH_PRIVATE(void)
th_request_async_read(th_socket* sock, th_allocator* allocator, th_request* request, th_io_handler* on_complete);

TH_PRIVATE(th_err)
th_request_store_cookie(th_request* request, th_string key, th_string value);

TH_PRIVATE(th_err)
th_request_store_header(th_request* request, th_string key, th_string value);

TH_PRIVATE(th_err)
th_request_store_query_param(th_request* request, th_string key, th_string value);

TH_PRIVATE(th_err)
th_request_store_body_param(th_request* request, th_string key, th_string value);

TH_PRIVATE(th_err)
th_request_store_path_param(th_request* request, th_string key, th_string value);

TH_PRIVATE(th_err)
th_request_store_uri_path(th_request* request, th_string path);

TH_PRIVATE(th_err)
th_request_store_uri_query(th_request* request, th_string query);

#endif
