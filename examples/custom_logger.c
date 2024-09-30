/** custom_logger example
 * @brief Demonstrates how to create a custom logger that writes log messages to a file.
 */

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
handler(void* userp, const th_request* req, th_response* resp)
{
    (void)userp;
    (void)req;
    th_set_body(resp, "Hello, Loggers!");
    th_add_header(resp, "Content-Type", "text/plain");
    return TH_ERR_OK;
}

typedef struct my_logger {
    th_log log;
    FILE* file;
} my_logger;

static void
my_log(void* self, int level, const char* msg)
{
    (void)level;
    my_logger* logger = self;
    fprintf(logger->file, "%s\n", msg);
}

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;
    signal(SIGINT, sigint_handler);

    // Initialize our custom logger
    my_logger logger = {
        .log = {
            .print = my_log,
        },
        .file = fopen("log.txt", "w"),
    };
    th_log_set(&logger.log);

    // Startup the server as usual
    th_server* server = NULL;
    th_err err = TH_ERR_OK;
    if ((err = th_server_create(&server, NULL)) != TH_ERR_OK)
        goto cleanup;
    if ((err = th_bind(server, "0.0.0.0", "8080", NULL)) != TH_ERR_OK)
        goto cleanup;
    if ((err = th_route(server, TH_METHOD_GET, "/", handler, NULL)) != TH_ERR_OK)
        goto cleanup;
    while (!stop) {
        th_poll(server, 1000);
    }
    fprintf(stderr, "Shutting down...\n");
cleanup:
    if (err != TH_ERR_OK) {
        fprintf(stderr, "Error: %s\n", th_strerror(err));
        return EXIT_FAILURE;
    }
    th_server_destroy(server);
    fclose(logger.file);
    return EXIT_SUCCESS;
}
