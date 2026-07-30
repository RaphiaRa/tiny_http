#ifndef TH_REQUEST_H
#define TH_REQUEST_H

#include <th.h>

#include "th_config.h"
#include "th_fcache.h"
#include "th_method.h"
#include "th_string.h"
#include "th_upload.h"
#include "th_vec.h"

struct th_iter_methods {
    bool (*next)(th_iter* it);
    const char* (*key)(const th_iter* it);
    const void* (*val)(const th_iter* it);
};

typedef struct th_hstr_pair {
    th_string key;
    th_string value;
} th_hstr_pair;

TH_INLINE(void)
th_hstr_pair_deinit(th_hstr_pair* pair)
{
    th_string_deinit(&pair->key);
    th_string_deinit(&pair->value);
}

TH_DEFINE_VEC(th_hstr_vec, th_hstr_pair, th_hstr_pair_deinit)

TH_DEFINE_VEC(th_upload_vec, th_upload, th_upload_deinit)

struct th_request {
    th_allocator* allocator;
    th_fcache* fcache;
    th_string uri_path;
    th_string uri_query;
    th_upload_vec uploads;
    th_hstr_vec cookies;
    th_hstr_vec headers;
    th_hstr_vec queryvars;
    th_hstr_vec formvars;
    th_hstr_vec pathvars;
    th_str body;
    th_method method;
    int version;
    bool close;
};

TH_PRIVATE(void)
th_request_init(th_request* request, th_fcache* fcache, th_allocator* allocator);

TH_PRIVATE(void)
th_request_deinit(th_request* request);

TH_PRIVATE(void)
th_request_reset(th_request* request);

TH_PRIVATE(void)
th_request_set_version(th_request* request, int version);

TH_PRIVATE(void)
th_request_set_method(th_request* request, th_method method);

TH_PRIVATE(th_err)
th_request_set_uri_path(th_request* request, th_str path);

TH_PRIVATE(th_err)
th_request_set_uri_query(th_request* request, th_str query);

TH_PRIVATE(th_err)
th_request_add_queryvar(th_request* request, th_str key, th_str value);

TH_PRIVATE(th_err)
th_request_add_formvar(th_request* request, th_str key, th_str value);

TH_PRIVATE(th_err)
th_request_add_pathvar(th_request* request, th_str key, th_str value);

TH_PRIVATE(th_err)
th_request_add_cookie(th_request* request, th_str key, th_str value);

TH_PRIVATE(th_err)
th_request_add_header(th_request* request, th_str key, th_str value);

TH_PRIVATE(th_err)
th_request_add_upload(th_request* request, th_str data, th_str name, th_str filename, th_str content_type);

TH_PRIVATE(void)
th_request_clear_queryvars(th_request* request);

TH_PRIVATE(void)
th_request_set_body(th_request* request, th_str body);

TH_PRIVATE(th_str)
th_request_get_header(th_request* request, th_str key) TH_MAYBE_UNUSED;

TH_PRIVATE(th_str)
th_request_get_pathvar(th_request* request, th_str key) TH_MAYBE_UNUSED;

TH_PRIVATE(th_str)
th_request_get_queryvar(th_request* request, th_str key) TH_MAYBE_UNUSED;

TH_PRIVATE(th_str)
th_request_get_formvar(th_request* request, th_str key) TH_MAYBE_UNUSED;

TH_PRIVATE(th_upload*)
th_request_get_upload(th_request* request, th_str key) TH_MAYBE_UNUSED;

#endif
