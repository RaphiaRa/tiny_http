#ifndef TH_REQUEST_H
#define TH_REQUEST_H

#include <th.h>

#include "th_config.h"
#include "th_hashmap.h"
#include "th_heap_string.h"
#include "th_method.h"

#define TH_HS_NULL ((th_heap_string){.impl.small.allocator = (void*)-1})
#define TH_HS_IS_NULL(hs) ((hs).impl.small.allocator == (void*)-1)

TH_INLINE(bool)
th_hs_eq(th_heap_string* a, th_heap_string* b)
{
    if (TH_HS_IS_NULL(*a) || TH_HS_IS_NULL(*b)) {
        return TH_HS_IS_NULL(*a) && TH_HS_IS_NULL(*b);
    }
    return th_heap_string_eq(a, th_heap_string_view(b));
}

#define TH_HS_EQ(a, b) th_hs_eq(&a, &b)
#define TH_HS_HASH(hs) th_heap_string_hash(&hs)
#define TH_HS_DEINIT(hs) th_heap_string_deinit(&hs)
TH_DEFINE_HASHMAP2(th_hs_map, th_heap_string, th_heap_string, TH_HS_HASH, TH_HS_EQ, TH_HS_NULL, TH_HS_DEINIT, TH_HS_DEINIT)

#define TH_HS_CSTR_EQ(a, b) (strcmp(th_heap_string_data(&a), b) == 0)
#define TH_HS_CSTR_HASH(s) th_cstr_hash(s)
TH_DEFINE_HASHMAP_FIND(th_hs_map, find_by_cstr, const char*, TH_HS_CSTR_HASH, TH_HS_CSTR_EQ, "")

struct th_request {
    th_allocator* allocator;
    th_heap_string uri_path;
    th_heap_string uri_query;
    th_hs_map cookies;
    th_hs_map headers;
    th_hs_map query_params;
    th_hs_map body_params;
    th_hs_map path_params;
    th_string body;
    th_method method;
    int version;
    bool close;
};

TH_PRIVATE(void)
th_request_init(th_request* request, th_allocator* allocator);

TH_PRIVATE(void)
th_request_deinit(th_request* request);

TH_PRIVATE(void)
th_request_reset(th_request* request);

TH_PRIVATE(void)
th_request_set_version(th_request* request, int version);

TH_PRIVATE(void)
th_request_set_method(th_request* request, th_method method);

TH_PRIVATE(th_err)
th_request_set_uri_path(th_request* request, th_string path);

TH_PRIVATE(th_err)
th_request_set_uri_query(th_request* request, th_string query);

TH_PRIVATE(th_err)
th_request_add_query_param(th_request* request, th_string key, th_string value);

TH_PRIVATE(th_err)
th_request_add_body_param(th_request* request, th_string key, th_string value);

TH_PRIVATE(th_err)
th_request_add_path_param(th_request* request, th_string key, th_string value);

TH_PRIVATE(th_err)
th_request_add_cookie(th_request* request, th_string key, th_string value);

TH_PRIVATE(th_err)
th_request_add_header(th_request* request, th_string key, th_string value);

TH_PRIVATE(void)
th_request_clear_query_params(th_request* request);

TH_PRIVATE(void)
th_request_set_body(th_request* request, th_string body);

#endif
