#include <th.h>

#include "th_clock.h"
#include "th_config.h"
#include "th_dir_mgr.h"
#include "th_file.h"
#include "th_filepath.h"
#include "th_listener.h"
#include "th_loop.h"
#include "th_poll.h"
#include "th_router.h"
#include "th_task.h"

struct th_server {
    th_reactor* reactor;
    th_loop loop;
    th_router router;
    th_dir_mgr dir_mgr;
    th_fcache fcache;
    th_listener* listeners;
    th_allocator* allocator;
};

TH_LOCAL(th_err)
th_server_init(th_server* server, th_allocator* allocator)
{
    th_router_init(&server->router, allocator);
    th_err err = TH_ERR_OK;
    th_loop_init(&server->loop, NULL);
    if ((err = th_poll_create(&server->reactor, &server->loop, allocator, th_clock_os(), th_pollops_os())) != TH_ERR_OK)
        goto cleanup_router;
    server->loop.reactor = server->reactor;
    th_dir_mgr_init(&server->dir_mgr, allocator);
    th_fcache_init(&server->fcache, th_file_ops_os(), allocator);
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
    th_loop_run(&server->loop);
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
    th_loop_deinit(&server->loop);
    th_reactor_destroy(server->reactor);
    th_router_deinit(&server->router);
    th_fcache_deinit(&server->fcache);
    th_dir_mgr_deinit(&server->dir_mgr);
}

TH_LOCAL(th_err)
th_server_bind(th_server* server, const char* host, const char* port, th_bind_opt* opt)
{
    th_listener* listener = NULL;
    th_err err = TH_ERR_OK;
    if ((err = th_listener_create(&listener, &server->loop,
                                  host, port,
                                  &server->router, &server->dir_mgr, &server->fcache,
                                  opt, server->allocator))
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
    return th_router_add_route(&server->router, method, th_str_from_cstr(path), handler, user_data);
}

TH_LOCAL(th_err)
th_server_route_ws(th_server* server, const char* path, th_ws_handler handler, void* user_data)
{
    return th_router_add_ws_route(&server->router, th_str_from_cstr(path), handler, user_data);
}

TH_LOCAL(th_err)
th_server_add_dir(th_server* server, const char* name, const char* path)
{
    th_dir dir;
    th_dir_init(&dir, th_dir_ops_os());
    th_err err = TH_ERR_OK;
    if ((err = th_dir_open(&dir, th_str_from_cstr(path))) != TH_ERR_OK) {
        th_dir_deinit(&dir);
        return err;
    }
    return th_dir_mgr_add(&server->dir_mgr, th_str_from_cstr(name), dir);
}

TH_LOCAL(th_err)
th_server_save_to_disk(th_server* server, th_buffer data, const char* dir_label, const char* filepath)
{
    th_dir* dir = th_dir_mgr_get(&server->dir_mgr, th_str_from_cstr(dir_label));
    if (!dir)
        return TH_ERR_HTTP(TH_CODE_NOT_FOUND);
    th_err err = TH_ERR_OK;
    th_filepath path;
    if ((err = th_filepath_init(&path, th_str_from_cstr(filepath))) != TH_ERR_OK)
        return err;
    th_open_opt opt = {.create = true, .write = true, .truncate = true};
    th_file file;
    th_file_init(&file, server->fcache.file_ops);
    if ((err = th_file_openat(&file, dir, &path, opt)) != TH_ERR_OK)
        return err;
    size_t total_written = 0;
    while (total_written < data.len) {
        size_t written = 0;
        if ((err = th_file_write(&file, data.ptr + total_written, data.len - total_written, total_written, &written))
            != TH_ERR_OK) {
            th_file_close(&file);
            return err;
        }
        total_written += written;
    }
    th_file_close(&file);
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_server_poll(th_server* server, int timeout_ms)
{
    return th_loop_poll(&server->loop, timeout_ms);
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
th_route_ws(th_server* server, const char* path, th_ws_handler handler, void* userp)
{
    return th_server_route_ws(server, path, handler, userp);
}

TH_PUBLIC(th_err)
th_add_dir(th_server* server, const char* name, const char* path)
{
    return th_server_add_dir(server, name, path);
}

TH_PUBLIC(th_err)
th_save_to_disk(th_server* server, th_buffer data, const char* dir_label, const char* filepath)
{
    return th_server_save_to_disk(server, data, dir_label, filepath);
}

TH_PUBLIC(th_err)
th_poll(th_server* server, int timeout_ms)
{
    return th_server_poll(server, timeout_ms);
}
