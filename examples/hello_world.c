#include <th.h>

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static th_server* server = NULL;
static bool running = true;

static void
sigint_handler(int signum)
{
    (void)signum;
    running = false;
}

static th_err
handler(void* userp, const th_request* req, th_response* resp)
{
    (void)userp;
    (void)req;
    th_set_body(resp, "Hello, World!");
    th_add_header(resp, "Content-Type", "text/plain");
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
    if ((err = th_route(server, TH_METHOD_GET, "/", handler, NULL)) != TH_ERR_OK)
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
