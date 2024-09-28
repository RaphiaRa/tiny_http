#include <th.h>

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static th_server* server = NULL;
static bool running = true;

static void sigint_handler(int signum)
{
    (void)signum;
    running = false;
}

const char* method_strings[] = {
    "GET",
    "POST",
    "PUT",
    "DELETE",
    "PATCH",
};

static th_err
handler(void* userp, const th_request* req, th_response* resp)
{
    (void)userp;
    size_t buf_len = 16 * 1024;
    char* buf = malloc(buf_len);
    size_t len = 0;
    len += (size_t)snprintf(buf, buf_len, "Method: %s\nPath: %s\nQuery: %s\nHeaders:\n",
                            method_strings[th_get_method(req)], th_get_path(req), th_get_query(req));
    th_map_iter iter = th_map_begin(th_get_headers(req));
    for (; iter; iter = th_map_next(th_get_headers(req), iter)) {
        len += (size_t)snprintf(buf + len, buf_len - len, "  %s: %s\n", iter->key, iter->value);
    }
    len += (size_t)snprintf(buf + len, buf_len - len, "Cookies:\n");
    iter = th_map_begin(th_get_cookies(req));
    for (; iter; iter = th_map_next(th_get_cookies(req), iter)) {
        len += (size_t)snprintf(buf + len, buf_len - len, "  %s: %s\n", iter->key, iter->value);
    }
    len += (size_t)snprintf(buf + len, buf_len - len, "Query params:\n");
    iter = th_map_begin(th_get_query_params(req));
    for (; iter; iter = th_map_next(th_get_query_params(req), iter)) {
        len += (size_t)snprintf(buf + len, buf_len - len, "  %s: %s\n", iter->key, iter->value);
    }
    len += (size_t)snprintf(buf + len, buf_len - len, "Body params:\n");
    iter = th_map_begin(th_get_body_params(req));
    for (; iter; iter = th_map_next(th_get_body_params(req), iter)) {
        len += (size_t)snprintf(buf + len, buf_len - len, "  %s: %s\n", iter->key, iter->value);
    }
    th_set_body(resp, buf);
    th_add_header(resp, "Content-Type", "text/plain; charset=utf-8");
    free(buf);
    return TH_ERR_OK;
}

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;
    signal(SIGINT, sigint_handler);

    th_err err = TH_ERR_OK;
    if ((err = th_server_create(&server, NULL)) != TH_ERR_OK)
        goto cleanup;
    if ((err = th_bind(server, "0.0.0.0", "8080", NULL)) != TH_ERR_OK)
        goto cleanup;
    if ((err = th_route(server, TH_METHOD_ANY, "/{path:path}", handler, NULL)) != TH_ERR_OK)
        goto cleanup;
    while (running) {
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
