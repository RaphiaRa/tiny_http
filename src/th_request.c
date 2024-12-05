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

/* hstr iterator begin */

TH_INLINE(bool)
th_hstr_iter_next(th_iter* it)
{
    it->ptr = ((const th_hstr_pair*)it->ptr) + 1;
    return it->ptr < it->end;
}

TH_INLINE(const char*)
th_hstr_iter_key(const th_iter* it)
{
    return th_heap_string_data(&((const th_hstr_pair*)it->ptr)->key);
}

TH_INLINE(const void*)
th_hstr_iter_val(const th_iter* it)
{
    return th_heap_string_data(&((const th_hstr_pair*)it->ptr)->value);
}

static th_iter_methods th_hstr_iter_methods = {
    .next = th_hstr_iter_next,
    .key = th_hstr_iter_key,
    .val = th_hstr_iter_val,
};

// hstr iterator end
// upload iterator begin

TH_INLINE(bool)
th_upload_iter_next(th_iter* it)
{
    it->ptr = ((const th_upload*)it->ptr) + 1;
    return it->ptr < it->end;
}

TH_INLINE(const char*)
th_upload_iter_key(const th_iter* it)
{
    return th_heap_string_data(&((const th_upload*)it->ptr)->name);
}

TH_INLINE(const void*)
th_upload_iter_val(const th_iter* it)
{
    return it->ptr;
}

static th_iter_methods th_upload_iter_methods = {
    .next = th_upload_iter_next,
    .key = th_upload_iter_key,
    .val = th_upload_iter_val,
};

// upload iterator end

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
th_request_add_upload(th_request* request, th_string data, th_string name, th_string filename, th_string content_type)
{
    th_upload upload;
    th_upload_init(&upload, data, request->fcache, request->allocator);
    th_err err = TH_ERR_OK;
    if ((err = th_upload_set_name(&upload, name)) != TH_ERR_OK)
        goto cleanup_upload;
    if ((err = th_upload_set_filename(&upload, filename)) != TH_ERR_OK)
        goto cleanup_upload;
    if ((err = th_upload_set_content_type(&upload, content_type)) != TH_ERR_OK)
        goto cleanup_upload;
    if ((err = th_upload_vec_push_back(&request->uploads, upload)) != TH_ERR_OK)
        goto cleanup_upload;
    return TH_ERR_OK;
cleanup_upload:
    th_upload_deinit(&upload);
    return err;
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
th_request_init(th_request* request, th_fcache* fcache, th_allocator* allocator)
{
    request->allocator = allocator ? allocator : th_default_allocator_get();
    request->fcache = fcache;
    th_heap_string_init(&request->uri_path, request->allocator);
    th_heap_string_init(&request->uri_query, request->allocator);
    th_upload_vec_init(&request->uploads, request->allocator);
    th_hstr_vec_init(&request->cookies, request->allocator);
    th_hstr_vec_init(&request->headers, request->allocator);
    th_hstr_vec_init(&request->queryvars, request->allocator);
    th_hstr_vec_init(&request->formvars, request->allocator);
    th_hstr_vec_init(&request->pathvars, request->allocator);
    request->body = (th_string){0};
    request->version = 0;
    request->close = false;
}

TH_PRIVATE(void)
th_request_deinit(th_request* request)
{
    th_heap_string_deinit(&request->uri_path);
    th_heap_string_deinit(&request->uri_query);
    th_upload_vec_deinit(&request->uploads);
    th_hstr_vec_deinit(&request->cookies);
    th_hstr_vec_deinit(&request->headers);
    th_hstr_vec_deinit(&request->queryvars);
    th_hstr_vec_deinit(&request->formvars);
    th_hstr_vec_deinit(&request->pathvars);
}

TH_PRIVATE(void)
th_request_reset(th_request* request)
{
    th_heap_string_clear(&request->uri_path);
    th_heap_string_clear(&request->uri_query);
    th_upload_vec_clear(&request->uploads);
    th_hstr_vec_clear(&request->cookies);
    th_hstr_vec_clear(&request->headers);
    th_hstr_vec_clear(&request->queryvars);
    th_hstr_vec_clear(&request->formvars);
    th_hstr_vec_clear(&request->pathvars);
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

TH_PRIVATE(th_string)
th_request_get_formvar(th_request* request, th_string key)
{
    return th_request_vec_get(&request->formvars, key);
}

TH_PRIVATE(th_upload*)
th_request_get_upload(th_request* request, th_string key)
{
    size_t num = th_upload_vec_size(&request->uploads);
    for (size_t i = 0; i < num; i++) {
        if (th_heap_string_eq(&request->uploads.data[i].name, key))
            return th_upload_vec_at(&request->uploads, i);
    }
    return NULL;
}

/* Public iterator API begin */

TH_PUBLIC(bool)
th_next(th_iter* it)
{
    return it->methods->next(it);
}

TH_PUBLIC(const char*)
th_key(const th_iter* it)
{
    return it->methods->key(it);
}

TH_PUBLIC(const void*)
th_val(const th_iter* it)
{
    return it->methods->val(it);
}

TH_PUBLIC(const char*)
th_cval(const th_iter* it)
{
    return (const char*)it->methods->val(it);
}

/* Public iterator API end */
/* Public request API begin */

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

TH_PUBLIC(th_buffer)
th_get_body(const th_request* req)
{
    return (th_buffer){req->body.ptr, req->body.len};
}

TH_PUBLIC(th_method)
th_get_method(const th_request* req)
{
    return req->method;
}

TH_PUBLIC(th_prot_version)
th_get_version(const th_request* req)
{
    return (th_prot_version)req->version;
}

TH_PUBLIC(const char*)
th_find_header(const th_request* req, const char* key)
{
    size_t num = th_hstr_vec_size(&req->headers);
    for (size_t i = 0; i < num; i++) {
        if (strncmp(key, th_heap_string_data(&req->headers.data[i].key), th_heap_string_len(&req->headers.data[i].key)) == 0) {
            return th_heap_string_data(&req->headers.data[i].value);
        }
    }
    return NULL;
}

TH_PUBLIC(th_iter)
th_header_iter(const th_request* req)
{
    return (th_iter){
        .methods = &th_hstr_iter_methods,
        .ptr = req->headers.data,
        .end = req->headers.data + req->headers.size,
    };
}

TH_PUBLIC(const char*)
th_find_cookie(const th_request* req, const char* key)
{
    size_t num = th_hstr_vec_size(&req->cookies);
    for (size_t i = 0; i < num; i++) {
        if (strncmp(key, th_heap_string_data(&req->cookies.data[i].key), th_heap_string_len(&req->cookies.data[i].key)) == 0) {
            return th_heap_string_data(&req->cookies.data[i].value);
        }
    }
    return NULL;
}

TH_PUBLIC(th_iter)
th_cookie_iter(const th_request* req)
{
    return (th_iter){
        .methods = &th_hstr_iter_methods,
        .ptr = req->cookies.data,
        .end = req->cookies.data + req->cookies.size,
    };
}

TH_PUBLIC(const char*)
th_find_queryvar(const th_request* req, const char* key)
{
    size_t num = th_hstr_vec_size(&req->queryvars);
    for (size_t i = 0; i < num; i++) {
        if (strncmp(key, th_heap_string_data(&req->queryvars.data[i].key), th_heap_string_len(&req->queryvars.data[i].key)) == 0) {
            return th_heap_string_data(&req->queryvars.data[i].value);
        }
    }
    return NULL;
}

TH_PUBLIC(th_iter)
th_queryvar_iter(const th_request* req)
{
    return (th_iter){
        .methods = &th_hstr_iter_methods,
        .ptr = req->queryvars.data,
        .end = req->queryvars.data + req->queryvars.size,
    };
}

TH_PUBLIC(const char*)
th_find_formvar(const th_request* req, const char* key)
{
    size_t num = th_hstr_vec_size(&req->formvars);
    for (size_t i = 0; i < num; i++) {
        if (strncmp(key, th_heap_string_data(&req->formvars.data[i].key), th_heap_string_len(&req->formvars.data[i].key)) == 0) {
            return th_heap_string_data(&req->formvars.data[i].value);
        }
    }
    return NULL;
}

TH_PUBLIC(th_iter)
th_formvar_iter(const th_request* req)
{
    return (th_iter){
        .methods = &th_hstr_iter_methods,
        .ptr = req->formvars.data,
        .end = req->formvars.data + req->formvars.size,
    };
}

TH_PUBLIC(const char*)
th_find_pathvar(const th_request* req, const char* key)
{
    size_t num = th_hstr_vec_size(&req->pathvars);
    for (size_t i = 0; i < num; i++) {
        if (strncmp(key, th_heap_string_data(&req->pathvars.data[i].key), th_heap_string_len(&req->pathvars.data[i].key)) == 0) {
            return th_heap_string_data(&req->pathvars.data[i].value);
        }
    }
    return NULL;
}

TH_PUBLIC(th_iter)
th_pathvar_iter(const th_request* req)
{
    return (th_iter){
        .methods = &th_hstr_iter_methods,
        .ptr = req->pathvars.data,
        .end = req->pathvars.data + req->pathvars.size,
    };
}

TH_PUBLIC(const th_upload*)
th_find_upload(const th_request* req, const char* name)
{
    size_t num = th_upload_vec_size(&req->uploads);
    for (size_t i = 0; i < num; i++) {
        if (strncmp(name, th_heap_string_data(&req->uploads.data[i].name), th_heap_string_len(&req->uploads.data[i].name)) == 0) {
            return th_upload_vec_cat(&req->uploads, i);
        }
    }
    return NULL;
}

TH_PUBLIC(th_iter)
th_upload_iter(const th_request* req)
{
    return (th_iter){
        .methods = &th_upload_iter_methods,
        .ptr = req->uploads.data,
        .end = req->uploads.data + req->uploads.size,
    };
}

/* Public request API end */
