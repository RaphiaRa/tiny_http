#include <th.h>

#include <assert.h>
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
handle_path(void* userp, const th_request* req, th_response* resp)
{
    (void)userp;
    return th_set_body_from_file(resp, "root", th_find_pathvar(req, "path"));
}

static th_err
handle_index(void* userp, const th_request* req, th_response* resp)
{
    (void)userp;
    (void)req;
    return th_set_body_from_file(resp, "root", "index.html");
}

static void
print_help(void)
{
    fprintf(stdout, "Possible arguments:\n");
    fprintf(stdout, "-r, --root <path>  Set the root directory\n");
    fprintf(stdout, "-p, --port <port>  Set the port\n");
    fprintf(stdout, "-k, --key <key>    Set the key file for SSL\n");
    fprintf(stdout, "-c, --cert <cert>  Set the cert file for SSL\n");
}

int main(int argc, char** argv)
{
    signal(SIGINT, sigint_handler);
    const char* root = NULL;
    const char* port = "8080";
    const char* key = NULL;
    const char* cert = NULL;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_help();
            return EXIT_SUCCESS;
        } else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--root") == 0) {
            root = argv[i + 1];
            ++i;
        } else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--port") == 0) {
            port = argv[i + 1];
            ++i;
        } else if (strcmp(argv[i], "-k") == 0 || strcmp(argv[i], "--key") == 0) {
            key = argv[i + 1];
            ++i;
        } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--cert") == 0) {
            cert = argv[i + 1];
            ++i;
        } else {
            fprintf(stderr, "Error: Unknown argument %s\n", argv[i]);
            print_help();
            return EXIT_FAILURE;
        }
    }
    if (root == NULL) {
        fprintf(stderr, "Error: Root directory not set\n");
        print_help();
        return EXIT_FAILURE;
    }
    th_server* server = NULL;
    th_err err = TH_ERR_OK;
    if ((err = th_server_create(&server, NULL)) != TH_ERR_OK) {
        fprintf(stderr, "Failed to create server: %s\n", th_strerror(err));
        return EXIT_FAILURE;
    }

    th_bind_opt opt = {0};
    opt.cert_file = cert;
    opt.key_file = key;
    if ((err = th_bind(server, "0.0.0.0", port, &opt)) != TH_ERR_OK)
        goto cleanup;
    if ((err = th_add_dir(server, "root", root)) != TH_ERR_OK)
        goto cleanup;
    if ((err = th_route(server, TH_METHOD_GET, "/{path:path}", handle_path, NULL)) != TH_ERR_OK)
        goto cleanup;
    if ((err = th_route(server, TH_METHOD_GET, "/", handle_index, NULL)) != TH_ERR_OK)
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
