// gcc ../benchmark/tiny_http_static.c th.c -O3 -DTH_CONFIG_MAX_CONNECTIONS=1024 -o tiny_http_static
#include <th.h>

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static th_err
handle_path(void* userp, const th_request* req, th_response* resp)
{
    (void)userp;
    const char* path = th_try_get_path_param(req, "path");
    return th_set_body_from_file(resp, "root", path);
}

static th_err
handle_index(void* userp, const th_request* req, th_response* resp)
{
    (void)userp;
    (void)req;
    return th_set_body_from_file(resp, "root", "index.html");
}

int main()
{
    th_err err = TH_ERR_OK;
    th_server* server = NULL;
    if ((err = th_server_create(&server, NULL)) != TH_ERR_OK)
        goto cleanup;
    if ((err = th_bind(server, "0.0.0.0", "8080", NULL)) != TH_ERR_OK)
        goto cleanup;
    if ((err = th_add_root(server, "root", "testfiles")) != TH_ERR_OK)
        goto cleanup;
    if ((err = th_route(server, TH_METHOD_GET, "/{path:path}", handle_path, NULL)) != TH_ERR_OK)
        goto cleanup;
    if ((err = th_route(server, TH_METHOD_GET, "/", handle_index, NULL)) != TH_ERR_OK)
        goto cleanup;
    while (1) {
        th_poll(server, 1000);
    }
    fprintf(stderr, "Shutting down...\n");
cleanup:
    if (err != TH_ERR_OK) {
        fprintf(stderr, "Error: %s\n", th_strerror(err));
        return EXIT_FAILURE;
    }
    th_server_destroy(server);
    return EXIT_SUCCESS;
}
