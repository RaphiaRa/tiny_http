#include <th.h>

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

static sig_atomic_t stop = 0;

static void
sigint_handler(int signum)
{
    stop = signum;
}

static th_err
ws_handler(void* userp, th_ws* ws, th_ws_event ev, th_buffer data, th_ws_type type)
{
    (void)userp;
    switch (ev) {
    case TH_WS_EVENT_OPEN:
        fprintf(stderr, "ws: open\n");
        break;
    case TH_WS_EVENT_DATA:
        fprintf(stderr, "ws: received %zu bytes: %.*s\n", data.len, (int)data.len, (const char*)data.ptr);
        th_ws_send(ws, data, type);
        break;
    case TH_WS_EVENT_CLOSE:
        fprintf(stderr, "ws: close\n");
        break;
    }
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
    if ((err = th_route_ws(server, "/ws", ws_handler, NULL)) != TH_ERR_OK)
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
