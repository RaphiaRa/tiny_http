#include "th_request.h"

#include "th_hashmap.h"
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
th_request_map_store(th_request* request, th_hs_map* map, th_string key, th_string value)
{
    th_err err = TH_ERR_OK;
    th_heap_string k;
    th_heap_string v;
    if ((err = th_heap_string_init_with(&k, key, request->allocator)) != TH_ERR_OK)
        return err;
    if ((err = th_heap_string_init_with(&v, value, request->allocator)) != TH_ERR_OK)
        goto cleanup_key;
    if ((err = th_hs_map_set(map, k, v)) != TH_ERR_OK)
        goto cleanup_value;
    return TH_ERR_OK;
cleanup_value:
    th_heap_string_deinit(&v);
cleanup_key:
    th_heap_string_deinit(&k);
    return err;
}

TH_LOCAL(th_err)
th_request_map_store_url_decoded(th_request* request, th_hs_map* map, th_string key, th_string value, th_url_decode_type type)
{
    th_err err = TH_ERR_OK;
    th_heap_string k;
    th_heap_string v;
    th_heap_string_init(&k, request->allocator);
    th_heap_string_init(&v, request->allocator);
    if ((err = th_url_decode_string(key, &k, type)) != TH_ERR_OK)
        return err;
    if ((err = th_url_decode_string(value, &v, type)) != TH_ERR_OK)
        goto cleanup_key;
    if ((err = th_hs_map_set(map, k, v)) != TH_ERR_OK)
        goto cleanup_value;
cleanup_value:
    th_heap_string_deinit(&v);
cleanup_key:
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
th_request_add_query_param(th_request* request, th_string key, th_string value)
{
    return th_request_map_store_url_decoded(request, &request->query_params, key, value, TH_URL_DECODE_TYPE_QUERY);
}

TH_PRIVATE(th_err)
th_request_add_body_param(th_request* request, th_string key, th_string value)
{
    return th_request_map_store_url_decoded(request, &request->body_params, key, value, TH_URL_DECODE_TYPE_QUERY);
}

TH_PRIVATE(th_err)
th_request_add_path_param(th_request* request, th_string key, th_string value)
{
    return th_request_map_store(request, &request->path_params, key, value);
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
th_request_clear_query_params(th_request* request)
{
    th_hs_map_reset(&request->query_params);
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
    th_hs_map_init(&request->cookies, request->allocator);
    th_hs_map_init(&request->headers, request->allocator);
    th_hs_map_init(&request->query_params, request->allocator);
    th_hs_map_init(&request->body_params, request->allocator);
    th_hs_map_init(&request->path_params, request->allocator);
    request->body = (th_string){0};
    request->version = 0;
    request->close = false;
}

TH_PRIVATE(void)
th_request_deinit(th_request* request)
{
    th_heap_string_deinit(&request->uri_path);
    th_heap_string_deinit(&request->uri_query);
    th_hs_map_deinit(&request->cookies);
    th_hs_map_deinit(&request->headers);
    th_hs_map_deinit(&request->query_params);
    th_hs_map_deinit(&request->body_params);
    th_hs_map_deinit(&request->path_params);
}

/* Public request API begin */

TH_PUBLIC(th_buffer)
th_get_body(const th_request* req)
{
    return (th_buffer){req->body.ptr, req->body.len};
}

TH_PUBLIC(const char*)
th_get_path(const th_request* req)
{
    return th_heap_string_data(&req->uri_path);
}

TH_PUBLIC(const char*)
th_get_query(const th_request* req)
{
    return th_heap_string_data(&req->uri_query);
}

TH_PUBLIC(const char*)
th_try_get_header(const th_request* req, const char* key)
{
    th_hs_map_iter iter = th_hs_map_find_by_cstr(&req->headers, key);
    if (iter == NULL)
        return NULL;
    return th_heap_string_data(&iter->value);
}

TH_PUBLIC(const char*)
th_try_get_cookie(const th_request* req, const char* key)
{
    th_hs_map_iter iter = th_hs_map_find_by_cstr(&req->cookies, key);
    if (iter == NULL)
        return NULL;
    return th_heap_string_data(&iter->value);
}

TH_PUBLIC(const char*)
th_try_get_query_param(const th_request* req, const char* key)
{
    th_hs_map_iter iter = th_hs_map_find_by_cstr(&req->query_params, key);
    if (iter == NULL)
        return NULL;
    return th_heap_string_data(&iter->value);
}

TH_PUBLIC(const char*)
th_try_get_body_param(const th_request* req, const char* key)
{
    th_hs_map_iter iter = th_hs_map_find_by_cstr(&req->body_params, key);
    if (iter == NULL)
        return NULL;
    return th_heap_string_data(&iter->value);
}

TH_PUBLIC(const char*)
th_try_get_path_param(const th_request* req, const char* key)
{
    th_hs_map_iter iter = th_hs_map_find_by_cstr(&req->path_params, key);
    if (iter == NULL)
        return NULL;
    return th_heap_string_data(&iter->value);
}

TH_PUBLIC(th_method)
th_get_method(const th_request* req)
{
    return req->method;
}

TH_PUBLIC(th_map*)
th_get_headers(const th_request* req)
{
    return (th_map*)&req->headers;
}

TH_PUBLIC(th_map*)
th_get_cookies(const th_request* req)
{
    return (th_map*)&req->cookies;
}

TH_PUBLIC(th_map*)
th_get_query_params(const th_request* req)
{
    return (th_map*)&req->query_params;
}

TH_PUBLIC(th_map*)
th_get_body_params(const th_request* req)
{
    return (th_map*)&req->body_params;
}

TH_PUBLIC(th_map*)
th_get_path_params(const th_request* req)
{
    return (th_map*)&req->path_params;
}

TH_PUBLIC(th_map_iter)
th_map_find(th_map* map, const char* key)
{
    return (th_map_iter)th_hs_map_find_by_cstr((th_hs_map*)map, key);
}

TH_PUBLIC(th_map_iter)
th_map_begin(th_map* map)
{
    return (th_map_iter)th_hs_map_begin((th_hs_map*)map);
}

TH_PUBLIC(th_map_iter)
th_map_next(th_map* map, th_map_iter iter)
{
    return (th_map_iter)th_hs_map_next((th_hs_map*)map, (th_hs_map_iter)iter);
}

TH_PUBLIC(const char*)
th_map_iter_key(th_map_iter iter)
{
    return th_heap_string_data(&((th_hs_map_iter)iter)->key);
}

TH_PUBLIC(const char*)
th_map_iter_value(th_map_iter iter)
{
    return th_heap_string_data(&((th_hs_map_iter)iter)->value);
}

/* Public request API end */
