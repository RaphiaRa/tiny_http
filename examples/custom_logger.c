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
handler(void* userp, const th_req* req, th_resp* resp)
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

static bool
my_logger_init(my_logger* log, const char* path)
{
    log->log.print = my_log;
    log->file = fopen(path, "w");
    return log->file != NULL;
}

static void
my_logger_deinit(my_logger* log)
{
    fclose(log->file);
}

int main(void)
{
    signal(SIGINT, sigint_handler);

    // Initialize our custom logger
    my_logger logger = {0};
    if (!my_logger_init(&logger, "custom_logger.log")) {
        fprintf(stderr, "Failed to open log file\n");
        return EXIT_FAILURE;
    }
    th_log_set(&logger.log);

    // Startup the server as usual
    th_server* server = NULL;
    th_err err = TH_ERR_OK;
    if ((err = th_server_create(&server, NULL)) != TH_ERR_OK)
        goto cleanup_logger;
    if ((err = th_bind(server, "0.0.0.0", "8080", NULL)) != TH_ERR_OK)
        goto cleanup_server;
    if ((err = th_route(server, TH_METHOD_GET, "/", handler, NULL)) != TH_ERR_OK)
        goto cleanup_server;
    while (!stop) {
        th_poll(server, 1000);
    }
    fprintf(stderr, "Shutting down...\n");
cleanup_server:
    th_server_destroy(server);
cleanup_logger:
    my_logger_deinit(&logger);
    if (err != TH_ERR_OK) {
        fprintf(stderr, "Error: %s\n", th_strerror(err));
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
