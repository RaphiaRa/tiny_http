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
handler(void* userp, const th_req* req, th_resp* resp)
{
    (void)userp;
    size_t buf_len = 16 * 1024;
    char* buf = malloc(buf_len);
    size_t len = 0;
    len += (size_t)snprintf(buf, buf_len, "Method: %s\nPath: %s\nQuery: %s\nHeaders:\n",
                            method_strings[req->method], req->path, req->query);
    for (size_t i = 0; i < req->num_headers; i++) {
        len += (size_t)snprintf(buf + len, buf_len - len, "  %s: %s\n",
                                req->headers[i].key, req->headers[i].value);
    }
    for (size_t i = 0; i < req->num_cookies; i++) {
        len += (size_t)snprintf(buf + len, buf_len - len, "  Cookie %zu: %s=%s\n",
                                i, req->cookies[i].key, req->cookies[i].value);
    }
    for (size_t i = 0; i < req->num_queryvars; i++) {
        len += (size_t)snprintf(buf + len, buf_len - len, "  Query var %zu: %s=%s\n",
                                i, req->queryvars[i].key, req->queryvars[i].value);
    }
    for (size_t i = 0; i < req->num_formvars; i++) {
        len += (size_t)snprintf(buf + len, buf_len - len, "  Form var %zu: %s=%s\n",
                                i, req->formvars[i].key, req->formvars[i].value);
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
