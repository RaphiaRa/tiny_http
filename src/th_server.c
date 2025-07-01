#include <th.h>

#include <stdint.h>
#include <string.h>

#include "th_align.h"
#include "th_config.h"
#include "th_context.h"
#include "th_dir_mgr.h"
#include "th_kqueue_service.h"
#include "th_listener.h"
#include "th_router.h"
#include "th_runner.h"
#include "th_task.h"

#define TH_MAIN_ALLOCATOR_PTR_OFFSET TH_ALIGNUP(sizeof(uint32_t), TH_ALIGNOF(th_max_align))
#define TH_MAIN_ALLOCATOR_BUCKET_NUM 5

typedef struct th_main_allocator {
    th_allocator base;
    th_allocator* allocator;
    th_pool_allocator pool[TH_MAIN_ALLOCATOR_BUCKET_NUM];
} th_main_allocator;

TH_LOCAL(size_t)
th_main_allocator_bucket_size(int index)
{
    TH_ASSERT(index >= 0 && index < TH_MAIN_ALLOCATOR_BUCKET_NUM);
    static const size_t bucket_sizes[] = {128, 256, 512, 1024, 2048};
    return bucket_sizes[index];
}

TH_LOCAL(int)
th_main_allocator_bucket_index(size_t size)
{
    static const int bucket_map[] = {0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4};
    size_t n = (size - 1) / 128;
    if (n < TH_ARRAY_SIZE(bucket_map))
        return bucket_map[n];
    return TH_MAIN_ALLOCATOR_BUCKET_NUM;
}

TH_LOCAL(void*)
th_main_allocator_alloc(void* self, size_t size)
{
    th_main_allocator* allocator = self;
    const size_t ptr_offset = TH_MAIN_ALLOCATOR_PTR_OFFSET;
    void* ptr = NULL;
    int index = th_main_allocator_bucket_index(size);
    if (index < TH_MAIN_ALLOCATOR_BUCKET_NUM) {
        ptr = th_allocator_alloc(&allocator->pool[index].base, th_main_allocator_bucket_size(index) + ptr_offset);
    } else {
        ptr = th_allocator_alloc(allocator->allocator, size + ptr_offset);
    }
    if (!ptr)
        return NULL;
    ((uint32_t*)ptr)[0] = (uint32_t)size;
    return (char*)ptr + ptr_offset;
}

TH_LOCAL(void)
th_main_allocator_free(void* self, void* ptr)
{
    th_main_allocator* allocator = self;
    const size_t ptr_offset = TH_MAIN_ALLOCATOR_PTR_OFFSET;
    void* old_ptr = (char*)ptr - ptr_offset;
    size_t size = ((uint32_t*)old_ptr)[0];
    int index = th_main_allocator_bucket_index(size);
    if (index < TH_MAIN_ALLOCATOR_BUCKET_NUM) {
        th_allocator_free(&allocator->pool[index].base, old_ptr);
    } else {
        th_allocator_free(allocator->allocator, old_ptr);
    }
}

TH_LOCAL(void*)
th_main_allocator_realloc(void* self, void* ptr, size_t size)
{
    th_main_allocator* allocator = self;
    if (!ptr)
        return th_main_allocator_alloc(allocator, size);
    const size_t ptr_offset = TH_MAIN_ALLOCATOR_PTR_OFFSET;
    void* old_ptr = (char*)ptr - ptr_offset;
    size_t old_size = ((uint32_t*)old_ptr)[0];
    if (old_size >= size)
        return ptr;
    void* new_ptr = th_main_allocator_alloc(allocator, size);
    if (!new_ptr)
        return NULL;
    memcpy(new_ptr, ptr, old_size);
    th_main_allocator_free(allocator, ptr);
    return new_ptr;
}

TH_LOCAL(void)
th_main_allocator_init(th_main_allocator* allocator, th_allocator* parent)
{
    allocator->base.alloc = th_main_allocator_alloc;
    allocator->base.realloc = th_main_allocator_realloc;
    allocator->base.free = th_main_allocator_free;
    allocator->allocator = parent;
    const size_t ptr_offset = TH_MAIN_ALLOCATOR_PTR_OFFSET;
    for (size_t i = 0; i < TH_MAIN_ALLOCATOR_BUCKET_NUM; ++i) {
        th_pool_allocator_init(&allocator->pool[i], parent, (1 << (i + 7)) + ptr_offset);
    }
}

TH_LOCAL(void)
th_main_allocator_deinit(th_main_allocator* allocator)
{
    for (size_t i = 0; i < TH_MAIN_ALLOCATOR_BUCKET_NUM; ++i) {
        th_pool_allocator_deinit(&allocator->pool[i]);
    }
}

struct th_server {
    th_context context;
    th_router router;
    th_fcache fcache;
    th_listener* listeners;
    th_allocator* allocator;
    th_main_allocator pool;
};

TH_LOCAL(th_err)
th_server_init(th_server* server, th_allocator* allocator)
{
    th_router_init(&server->router, allocator);
    th_err err = TH_ERR_OK;
    if ((err = th_context_init(&server->context, allocator)) != TH_ERR_OK)
        goto cleanup_router;
    th_fcache_init(&server->fcache, allocator);
    th_main_allocator_init(&server->pool, allocator);
    server->listeners = NULL;
    server->allocator = allocator;
cleanup_router:
    th_router_deinit(&server->router);
    return err;
}

TH_LOCAL(void)
th_server_stop(th_server* server)
{
    th_listener* listener = server->listeners;
    while (listener) {
        th_listener_stop(listener);
        listener = listener->next;
    }
    th_context_drain(&server->context);
}

TH_LOCAL(void)
th_server_deinit(th_server* server)
{
    th_listener* listener = server->listeners;
    while (listener) {
        th_listener* next = listener->next;
        th_listener_destroy(listener);
        listener = next;
    }
    th_context_deinit(&server->context);
    th_router_deinit(&server->router);
    th_fcache_deinit(&server->fcache);
    th_main_allocator_deinit(&server->pool);
}

TH_LOCAL(th_err)
th_server_bind(th_server* server, const char* host, const char* port, th_bind_opt* opt)
{
    th_listener* listener = NULL;
    th_err err = TH_ERR_OK;
    if ((err = th_listener_create(&listener, &server->context,
                                  host, port,
                                  &server->router, &server->fcache,
                                  opt, &server->pool.base))
        != TH_ERR_OK) {
        return err;
    }
    if ((err = th_listener_start(listener)) != TH_ERR_OK) {
        th_listener_destroy(listener);
        return err;
    }
    listener->next = server->listeners;
    server->listeners = listener;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_server_route(th_server* server, th_method method, const char* path, th_handler handler, void* user_data)
{
    return th_router_add_route(&server->router, method, th_string_from_cstr(path), handler, user_data);
}

TH_LOCAL(th_err)
th_server_add_dir(th_server* server, const char* name, const char* path)
{
    return th_fcache_add_dir(&server->fcache, th_string_from_cstr(name), th_string_from_cstr(path));
}

TH_LOCAL(th_err)
th_server_poll(th_server* server, int timeout_ms)
{
    return th_context_poll(&server->context, timeout_ms);
}

/* public server API */

TH_PUBLIC(th_err)
th_server_create(th_server** out, th_allocator* allocator)
{
    allocator = allocator ? allocator : th_default_allocator_get();
    th_server* server = th_allocator_alloc(allocator, sizeof(th_server));
    if (!server)
        return TH_ERR_BAD_ALLOC;
    th_err err = TH_ERR_OK;
    if ((err = th_server_init(server, allocator)) != TH_ERR_OK) {
        th_allocator_free(server->allocator, server);
        return err;
    }
    *out = server;
    return TH_ERR_OK;
}

TH_PUBLIC(void)
th_server_destroy(th_server* server)
{
    th_server_stop(server);
    th_server_deinit(server);
    th_allocator_free(server->allocator, server);
}

TH_PUBLIC(th_err)
th_bind(th_server* server, const char* addr, const char* port, th_bind_opt* opt)
{
    return th_server_bind(server, addr, port, opt);
}

TH_PUBLIC(th_err)
th_route(th_server* server, th_method method, const char* route, th_handler handler, void* userp)
{
    return th_server_route(server, method, route, handler, userp);
}

TH_PUBLIC(th_err)
th_add_dir(th_server* server, const char* name, const char* path)
{
    return th_server_add_dir(server, name, path);
}

TH_PUBLIC(th_err)
th_poll(th_server* server, int timeout_ms)
{
    return th_server_poll(server, timeout_ms);
}
