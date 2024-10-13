#ifndef TH_REQUEST_H
#define TH_REQUEST_H

#include <th.h>

#include "th_config.h"
#include "th_heap_string.h"
#include "th_method.h"
#include "th_vec.h"

typedef struct th_hstr_pair {
    th_heap_string key;
    th_heap_string value;
} th_hstr_pair;

TH_INLINE(void)
th_hstr_pair_deinit(th_hstr_pair* pair)
{
    th_heap_string_deinit(&pair->key);
    th_heap_string_deinit(&pair->value);
}

TH_DEFINE_VEC(th_hstr_vec, th_hstr_pair, th_hstr_pair_deinit)

TH_DEFINE_VEC(th_keyval_vec, th_keyval, (void))

TH_DEFINE_VEC(th_upload_vec, th_upload, (void))

typedef struct th_request {
    th_allocator* allocator;
    th_heap_string uri_path;
    th_heap_string uri_query;
    th_upload_vec uploads;
    th_hstr_vec cookies;
    th_hstr_vec headers;
    th_hstr_vec queryvars;
    th_hstr_vec formvars;
    th_hstr_vec pathvars;
    th_keyval_vec keyvals;
    th_string body;
    th_method method;
    int version;
    bool close;
} th_request;

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
th_request_add_queryvar(th_request* request, th_string key, th_string value);

TH_PRIVATE(th_err)
th_request_add_formvar(th_request* request, th_string key, th_string value);

TH_PRIVATE(th_err)
th_request_add_pathvar(th_request* request, th_string key, th_string value);

TH_PRIVATE(th_err)
th_request_add_cookie(th_request* request, th_string key, th_string value);

TH_PRIVATE(th_err)
th_request_add_header(th_request* request, th_string key, th_string value);

TH_PRIVATE(th_err)
th_request_add_upload(th_request* request, th_upload upload) TH_MAYBE_UNUSED;

TH_PRIVATE(void)
th_request_clear_queryvars(th_request* request);

TH_PRIVATE(void)
th_request_set_body(th_request* request, th_string body);

TH_PRIVATE(th_string)
th_request_get_header(th_request* request, th_string key) TH_MAYBE_UNUSED;

TH_PRIVATE(th_string)
th_request_get_pathvar(th_request* request, th_string key) TH_MAYBE_UNUSED;

TH_PRIVATE(th_string)
th_request_get_queryvar(th_request* request, th_string key) TH_MAYBE_UNUSED;

TH_PRIVATE(th_err)
th_request_setup_public(th_request* request, th_req* public_request);

#endif
