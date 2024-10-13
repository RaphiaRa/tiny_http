#include "th_request.h"

#include "th_log.h"
#include "th_method.h"
#include "th_string.h"
#include "th_url_decode.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#undef TH_LOG_TAG
#define TH_LOG_TAG "request"

TH_LOCAL(th_err)
th_request_map_store(th_request* request, th_hstr_vec* vec, th_string key, th_string value)
{
    th_err err = TH_ERR_OK;
    th_heap_string k;
    th_heap_string v;
    if ((err = th_heap_string_init_with(&k, key, request->allocator)) != TH_ERR_OK)
        return err;
    if ((err = th_heap_string_init_with(&v, value, request->allocator)) != TH_ERR_OK)
        goto cleanup_key;
    if ((err = th_hstr_vec_push_back(vec, (th_hstr_pair){k, v})) != TH_ERR_OK)
        goto cleanup_value;
    return TH_ERR_OK;
cleanup_value:
    th_heap_string_deinit(&v);
cleanup_key:
    th_heap_string_deinit(&k);
    return err;
}

TH_LOCAL(th_err)
th_request_map_store_url_decoded(th_request* request, th_hstr_vec* vec, th_string key, th_string value, th_url_decode_type type)
{
    th_err err = TH_ERR_OK;
    th_heap_string k;
    th_heap_string v;
    th_heap_string_init(&k, request->allocator);
    th_heap_string_init(&v, request->allocator);
    if ((err = th_url_decode_string(key, &k, type)) != TH_ERR_OK)
        goto cleanup;
    if ((err = th_url_decode_string(value, &v, type)) != TH_ERR_OK)
        goto cleanup;
    if ((err = th_hstr_vec_push_back(vec, (th_hstr_pair){k, v})) != TH_ERR_OK)
        goto cleanup;
    return TH_ERR_OK;
cleanup:
    th_heap_string_deinit(&v);
    th_heap_string_deinit(&k);
    return err;
}

TH_PRIVATE(th_err)
th_request_add_cookie(th_request* request, th_string key, th_string value)
{
    return th_request_map_store(request, &request->cookies, key, value);
}

TH_PRIVATE(th_err)
th_request_add_header(th_request* request, th_string key, th_string value)
{
    return th_request_map_store(request, &request->headers, key, value);
}

TH_PRIVATE(th_err)
th_request_add_upload(th_request* request, th_upload upload)
{
    return th_upload_vec_push_back(&request->uploads, upload);
}

TH_PRIVATE(th_err)
th_request_add_queryvar(th_request* request, th_string key, th_string value)
{
    return th_request_map_store_url_decoded(request, &request->queryvars, key, value, TH_URL_DECODE_TYPE_QUERY);
}

TH_PRIVATE(th_err)
th_request_add_formvar(th_request* request, th_string key, th_string value)
{
    return th_request_map_store_url_decoded(request, &request->formvars, key, value, TH_URL_DECODE_TYPE_QUERY);
}

TH_PRIVATE(th_err)
th_request_add_pathvar(th_request* request, th_string key, th_string value)
{
    return th_request_map_store(request, &request->pathvars, key, value);
}

TH_PRIVATE(th_err)
th_request_set_uri_path(th_request* request, th_string path)
{
    return th_heap_string_set(&request->uri_path, path);
}

TH_PRIVATE(th_err)
th_request_set_uri_query(th_request* request, th_string query)
{
    return th_heap_string_set(&request->uri_query, query);
}

TH_PRIVATE(void)
th_request_set_version(th_request* request, int version)
{
    request->version = version;
}

TH_PRIVATE(void)
th_request_set_method(th_request* request, th_method method)
{
    request->method = method;
}

TH_PRIVATE(void)
th_request_clear_queryvars(th_request* request)
{
    th_hstr_vec_clear(&request->queryvars);
    // TODO: clear heap strings
}

TH_PRIVATE(void)
th_request_set_body(th_request* request, th_string body)
{
    request->body = body;
}

TH_PRIVATE(void)
th_request_init(th_request* request, th_allocator* allocator)
{
    request->allocator = allocator ? allocator : th_default_allocator_get();
    th_heap_string_init(&request->uri_path, request->allocator);
    th_heap_string_init(&request->uri_query, request->allocator);
    th_hstr_vec_init(&request->cookies, request->allocator);
    th_hstr_vec_init(&request->headers, request->allocator);
    th_hstr_vec_init(&request->queryvars, request->allocator);
    th_hstr_vec_init(&request->formvars, request->allocator);
    th_hstr_vec_init(&request->pathvars, request->allocator);
    th_keyval_vec_init(&request->keyvals, request->allocator);
    request->body = (th_string){0};
    request->version = 0;
    request->close = false;
}

TH_PRIVATE(void)
th_request_deinit(th_request* request)
{
    th_heap_string_deinit(&request->uri_path);
    th_heap_string_deinit(&request->uri_query);
    th_hstr_vec_deinit(&request->cookies);
    th_hstr_vec_deinit(&request->headers);
    th_hstr_vec_deinit(&request->queryvars);
    th_hstr_vec_deinit(&request->formvars);
    th_hstr_vec_deinit(&request->pathvars);
    th_keyval_vec_deinit(&request->keyvals);
}

TH_PRIVATE(void)
th_request_reset(th_request* request)
{
    th_heap_string_clear(&request->uri_path);
    th_heap_string_clear(&request->uri_query);
    th_hstr_vec_clear(&request->cookies);
    th_hstr_vec_clear(&request->headers);
    th_hstr_vec_clear(&request->queryvars);
    th_hstr_vec_clear(&request->formvars);
    th_hstr_vec_clear(&request->pathvars);
    th_keyval_vec_clear(&request->keyvals);
    request->body = (th_string){0};
    request->version = 0;
    request->close = false;
}

TH_LOCAL(th_string)
th_request_vec_get(th_hstr_vec* vec, th_string key)
{
    size_t num = th_hstr_vec_size(vec);
    for (size_t i = 0; i < num; i++) {
        if (th_heap_string_eq(&vec->data[i].key, key))
            return th_heap_string_view(&vec->data[i].value);
    }
    return TH_STRING("");
}

TH_PRIVATE(th_string)
th_request_get_header(th_request* request, th_string key)
{
    return th_request_vec_get(&request->headers, key);
}

TH_PRIVATE(th_string)
th_request_get_pathvar(th_request* request, th_string key)
{
    return th_request_vec_get(&request->pathvars, key);
}

TH_PRIVATE(th_string)
th_request_get_queryvar(th_request* request, th_string key)
{
    return th_request_vec_get(&request->queryvars, key);
}

TH_LOCAL(size_t)
th_request_setup_vec(th_request* request, const th_keyval** keyval, size_t* num_keyval, th_hstr_vec* hstr_vec, size_t pos)
{
    size_t num = th_hstr_vec_size(hstr_vec);
    for (size_t i = 0; i < num; i++) {
        th_keyval* keyval = th_keyval_vec_at(&request->keyvals, pos + i);
        keyval->key = th_heap_string_data(&hstr_vec->data[i].key);
        keyval->value = th_heap_string_data(&hstr_vec->data[i].value);
    }
    *keyval = th_keyval_vec_at(&request->keyvals, pos);
    *num_keyval = num;
    return num;
}

TH_PRIVATE(th_err)
th_request_setup_public(th_request* request, th_req* pub)
{
    pub->path = th_heap_string_data(&request->uri_path);
    pub->query = th_heap_string_data(&request->uri_query);
    pub->uploads = th_upload_vec_at(&request->uploads, 0);
    pub->num_uploads = th_upload_vec_size(&request->uploads);
    size_t num_keyvals = th_hstr_vec_size(&request->cookies)
                         + th_hstr_vec_size(&request->headers)
                         + th_hstr_vec_size(&request->queryvars)
                         + th_hstr_vec_size(&request->formvars)
                         + th_hstr_vec_size(&request->pathvars);
    th_err err = TH_ERR_OK;
    if ((err = th_keyval_vec_resize(&request->keyvals, num_keyvals)) != TH_ERR_OK)
        return err;
    size_t pos = 0;
    pos += th_request_setup_vec(request, &pub->cookies, &pub->num_cookies, &request->cookies, pos);
    pos += th_request_setup_vec(request, &pub->headers, &pub->num_headers, &request->headers, pos);
    pos += th_request_setup_vec(request, &pub->queryvars, &pub->num_queryvars, &request->queryvars, pos);
    pos += th_request_setup_vec(request, &pub->formvars, &pub->num_formvars, &request->formvars, pos);
    pos += th_request_setup_vec(request, &pub->pathvars, &pub->num_pathvars, &request->pathvars, pos);
    pub->body = (th_buffer){request->body.ptr, request->body.len};
    pub->method = request->method;
    pub->version = request->version;
    return TH_ERR_OK;
}

/* Public request API begin */

TH_PUBLIC(const char*)
th_find_header(const th_req* req, const char* key)
{
    size_t num = req->num_headers;
    for (size_t i = 0; i < num; i++) {
        if (*key == *req->headers[i].key && strcmp(req->headers[i].key, key) == 0) {
            return req->headers[i].value;
        }
    }
    return NULL;
}

TH_PUBLIC(const char*)
th_find_cookie(const th_req* req, const char* key)
{
    size_t num = req->num_cookies;
    for (size_t i = 0; i < num; i++) {
        if (*key == *req->cookies[i].key && strcmp(req->cookies[i].key, key) == 0) {
            return req->cookies[i].value;
        }
    }
    return NULL;
}

TH_PUBLIC(const char*)
th_find_queryvar(const th_req* req, const char* key)
{
    size_t num = req->num_queryvars;
    for (size_t i = 0; i < num; i++) {
        if (*key == *req->queryvars[i].key && strcmp(req->queryvars[i].key, key) == 0) {
            return req->queryvars[i].value;
        }
    }
    return NULL;
}

TH_PUBLIC(const char*)
th_find_formvar(const th_req* req, const char* key)
{
    size_t num = req->num_formvars;
    for (size_t i = 0; i < num; i++) {
        if (*key == *req->formvars[i].key && strcmp(req->formvars[i].key, key) == 0) {
            return req->formvars[i].value;
        }
    }
    return NULL;
}

TH_PUBLIC(const char*)
th_find_pathvar(const th_req* req, const char* key)
{
    size_t num = req->num_pathvars;
    for (size_t i = 0; i < num; i++) {
        if (*key == *req->pathvars[i].key && strcmp(req->pathvars[i].key, key) == 0) {
            return req->pathvars[i].value;
        }
    }
    return NULL;
}

/* Public request API end */
