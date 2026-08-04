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

static th_err
handler(void* userp, const th_request* req, th_response* resp)
{
    (void)userp;
    for (th_iter it = th_part_iter(req); th_next(&it);) {
        const th_part* part = th_val(&it);
        const char* filename = th_part_filename(part);
        if (filename[0] == '\0')
            continue; // plain form field, not a file
        th_err err = TH_ERR_OK;
        if ((err = th_save_to_disk(req, th_part_content(part), "upload_dir", filename)) != TH_ERR_OK) {
            th_printf_body(resp, "Failed to save upload: %s, reason %s\n", filename, th_strerror(err));
            th_add_header(resp, "Content-Type", "text/plain");
            return TH_ERR_OK;
        }
    }
    th_set_body(resp, "Uploads saved successfully!\n");
    th_add_header(resp, "Content-Type", "text/plain");
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
    if ((err = th_add_dir(server, "upload_dir", "./uploads")) != TH_ERR_OK)
        goto cleanup;
    if ((err = th_route(server, TH_METHOD_POST, "/", handler, NULL)) != TH_ERR_OK)
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
