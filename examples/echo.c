#include <th.h>

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static sig_atomic_t stop = 0;

static void
sigint_handler(int signum)
{
    stop = signum;
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
    for (th_iter it = th_header_iter(req); th_next(&it);) {
        len += (size_t)snprintf(buf + len, buf_len - len, "  %s: %s\n", th_key(&it), th_cval(&it));
    }
    for (th_iter it = th_cookie_iter(req); th_next(&it);) {
        len += (size_t)snprintf(buf + len, buf_len - len, "  Cookie %s: %s\n", th_key(&it), th_cval(&it));
    }
    for (th_iter it = th_queryvar_iter(req); th_next(&it);) {
        len += (size_t)snprintf(buf + len, buf_len - len, "  Query var %s: %s\n", th_key(&it), th_cval(&it));
    }
    for (th_iter it = th_formvar_iter(req); th_next(&it);) {
        len += (size_t)snprintf(buf + len, buf_len - len, "  Form var %s: %s\n", th_key(&it), th_cval(&it));
    }
    th_set_body(resp, buf);
    th_add_header(resp, "Content-Type", "text/plain; charset=utf-8");
    free(buf);
    return TH_ERR_OK;
}

int main(void)
{
    signal(SIGINT, sigint_handler);
    th_server* server = NULL;
    th_err err = TH_ERR_OK;
    if ((err = th_server_create(&server, NULL)) != TH_ERR_OK) {
        fprintf(stderr, "Failed to create server: %s\n", th_strerror(err));
        return EXIT_FAILURE;
    }
    if ((err = th_bind(server, "0.0.0.0", "8080", NULL)) != TH_ERR_OK)
        goto cleanup;
    if ((err = th_route(server, TH_METHOD_ANY, "/{path:path}", handler, NULL)) != TH_ERR_OK)
        goto cleanup;
    while (!stop) {
        th_poll(server, 1000);
    }
    fprintf(stderr, "Shutting down...\n");
cleanup:
    th_server_destroy(server);
    if (err != TH_ERR_OK) {
        fprintf(stderr, "Error: %s\n", th_strerror(err));
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
