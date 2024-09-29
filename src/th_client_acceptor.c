#include "th_client_acceptor.h"
#include "th_client.h"
#include "th_config.h"
#include "th_log.h"
#include "th_utility.h"

#undef TH_LOG_TAG
#define TH_LOG_TAG "client_acceptor"

TH_PRIVATE(th_err)
th_client_acceptor_init(th_client_acceptor* client_acceptor,
                        th_context* context,
                        th_router* router, th_fcache* fcache,
                        th_acceptor* acceptor, th_allocator* allocator)
{
    client_acceptor->context = context;
    client_acceptor->router = router;
    client_acceptor->fcache = fcache;
    client_acceptor->acceptor = acceptor;
    client_acceptor->running = 0;
    client_acceptor->ssl_enabled = 0;
    client_acceptor->client = NULL;
    client_acceptor->allocator = allocator ? allocator : th_default_allocator_get();
    th_client_tracker_init(&client_acceptor->client_tracker);
    return TH_ERR_OK;
}

TH_PRIVATE(th_err)
th_client_acceptor_enable_ssl(th_client_acceptor* client_acceptor, const char* key_file, const char* cert_file)
{
#if TH_WITH_SSL
    th_err err = TH_ERR_OK;
    if ((err = th_ssl_context_init(&client_acceptor->ssl_context, key_file, cert_file)) != TH_ERR_OK) {
        return err;
    }
    client_acceptor->ssl_enabled = 1;
    return TH_ERR_OK;
#else
    (void)client_acceptor;
    (void)key_file;
    (void)cert_file;
    TH_LOG_ERROR("SSL is not not enabled in this build.");
    return TH_ERR_NOSUPPORT;
#endif
}

TH_LOCAL(th_err)
th_client_acceptor_do_accept_tcp(th_client_acceptor* client_acceptor)
{
    th_err err = TH_ERR_OK;
    if ((err = th_tcp_client_create(&client_acceptor->client, client_acceptor->context,
                                    client_acceptor->router, client_acceptor->fcache,
                                    (th_client_observer*)&client_acceptor->client_tracker,
                                    client_acceptor->allocator))
        != TH_ERR_OK) {
        return err;
    }
    th_acceptor_async_accept(client_acceptor->acceptor,
                             th_client_get_address(client_acceptor->client),
                             &client_acceptor->accept_handler.base);
    return TH_ERR_OK;
}

TH_LOCAL(void)
th_client_acceptor_tcp_client_destroy_handler_fn(void* self)
{
    th_client_acceptor* client_acceptor = self;
    if (!client_acceptor->running)
        return;
    th_err err = TH_ERR_OK;
    if ((err = th_client_acceptor_do_accept_tcp(client_acceptor)) != TH_ERR_OK) {
        TH_LOG_ERROR("Failed to initiate accept: %s, try again later", th_strerror(err));
        th_client_tracker_async_wait(&client_acceptor->client_tracker, &client_acceptor->client_destroy_handler.base);
    }
}

TH_LOCAL(void)
th_client_acceptor_accept_handler_fn(void* self, size_t result, th_err err)
{
    th_client_acceptor_accept_handler* handler = self;
    th_client_acceptor* client_acceptor = handler->client_acceptor;
    if (err != TH_ERR_OK) {
        TH_LOG_ERROR("Accept failed: %s", th_strerror(err));
        th_client_unref(client_acceptor->client);
    } else if (err == TH_ERR_OK) {
        th_socket_set_fd(th_client_get_socket(client_acceptor->client), (int)result);
        if (th_client_tracker_count(&client_acceptor->client_tracker) > TH_CONFIG_MAX_CONNECTIONS) {
            TH_LOG_WARN("Too many connections, rejecting new connection");
            th_client_set_mode(client_acceptor->client, TH_EXCHANGE_MODE_REJECT_UNAVAILABLE);
        }
        th_client_start(client_acceptor->client);
    }
    if (!client_acceptor->running) {
        return;
    }
    if ((err = th_client_acceptor_do_accept_tcp(client_acceptor)) != TH_ERR_OK) {
        TH_LOG_ERROR("Failed to initiate accept: %s, try again later", th_strerror(err));
        th_client_tracker_async_wait(&client_acceptor->client_tracker, &client_acceptor->client_destroy_handler.base);
    }
}

TH_LOCAL(th_err)
th_client_acceptor_start_tcp(th_client_acceptor* client_acceptor)
{
    // Accept handler
    client_acceptor->accept_handler.client_acceptor = client_acceptor;
    th_io_handler_init(&client_acceptor->accept_handler.base, th_client_acceptor_accept_handler_fn, NULL);
    // Client destroy handler
    client_acceptor->client_destroy_handler.client_acceptor = client_acceptor;
    th_task_init(&client_acceptor->client_destroy_handler.base, th_client_acceptor_tcp_client_destroy_handler_fn, NULL);
    client_acceptor->running = 1;
    th_err err = TH_ERR_OK;
    if ((err = th_client_acceptor_do_accept_tcp(client_acceptor)) != TH_ERR_OK) {
        return err;
    }
    return TH_ERR_OK;
}

#if TH_WITH_SSL
TH_LOCAL(th_err)
th_client_acceptor_do_accept_ssl(th_client_acceptor* client_acceptor)
{
    th_err err = TH_ERR_OK;
    if ((err = th_ssl_client_create(&client_acceptor->client, client_acceptor->context,
                                    &client_acceptor->ssl_context,
                                    client_acceptor->router, client_acceptor->fcache,
                                    (th_client_observer*)&client_acceptor->client_tracker,
                                    client_acceptor->allocator))
        != TH_ERR_OK) {
        return err;
    }
    th_acceptor_async_accept(client_acceptor->acceptor,
                             th_client_get_address(client_acceptor->client),
                             &client_acceptor->accept_handler.base);
    return TH_ERR_OK;
}

TH_LOCAL(void)
th_client_acceptor_ssl_client_destroy_handler_fn(void* self)
{
    th_client_acceptor* client_acceptor = self;
    if (!client_acceptor->running)
        return;
    th_err err = TH_ERR_OK;
    if ((err = th_client_acceptor_do_accept_ssl(client_acceptor)) != TH_ERR_OK) {
        TH_LOG_ERROR("Failed to initiate accept: %s, try again later", th_strerror(err));
        th_client_tracker_async_wait(&client_acceptor->client_tracker, &client_acceptor->client_destroy_handler.base);
    }
}

TH_LOCAL(void)
th_client_acceptor_ssl_accept_handler_fn(void* self, size_t result, th_err err)
{
    th_client_acceptor_accept_handler* handler = self;
    th_client_acceptor* client_acceptor = handler->client_acceptor;
    if (err != TH_ERR_OK) {
        TH_LOG_ERROR("Accept failed: %s", th_strerror(err));
        th_client_unref(client_acceptor->client);
    } else if (err == TH_ERR_OK) {
        th_socket_set_fd(th_client_get_socket(client_acceptor->client), (int)result);
        if (th_client_tracker_count(&client_acceptor->client_tracker) > TH_CONFIG_MAX_CONNECTIONS) {
            TH_LOG_WARN("Too many connections, rejecting new connection");
            th_client_set_mode(client_acceptor->client, TH_EXCHANGE_MODE_REJECT_UNAVAILABLE);
        }
        th_client_start(client_acceptor->client);
    }
    if (!client_acceptor->running) {
        return;
    }
    if ((err = th_client_acceptor_do_accept_ssl(client_acceptor)) != TH_ERR_OK) {
        TH_LOG_ERROR("Failed to initiate accept: %s, try again later", th_strerror(err));
        th_client_tracker_async_wait(&client_acceptor->client_tracker, &client_acceptor->client_destroy_handler.base);
    }
}

TH_LOCAL(th_err)
th_client_acceptor_start_ssl(th_client_acceptor* client_acceptor)
{
    // Accept handler
    client_acceptor->accept_handler.client_acceptor = client_acceptor;
    th_io_handler_init(&client_acceptor->accept_handler.base, th_client_acceptor_ssl_accept_handler_fn, NULL);
    // Client destroy handler
    client_acceptor->client_destroy_handler.client_acceptor = client_acceptor;
    th_task_init(&client_acceptor->client_destroy_handler.base, th_client_acceptor_ssl_client_destroy_handler_fn, NULL);
    client_acceptor->running = 1;
    th_err err = TH_ERR_OK;
    if ((err = th_client_acceptor_do_accept_ssl(client_acceptor)) != TH_ERR_OK) {
        return err;
    }
    return TH_ERR_OK;
}
#endif /* TH_WITH_SSL */

TH_PRIVATE(th_err)
th_client_acceptor_start(th_client_acceptor* client_acceptor)
{
    if (client_acceptor->ssl_enabled) {
#if TH_WITH_SSL
        return th_client_acceptor_start_ssl(client_acceptor);
#else  /* TH_WITH_SSL */
        TH_ASSERT(0 && "SSL is not enabled in this build.");
        return TH_ERR_NOSUPPORT;
#endif /* TH_WITH_SSL */
    } else {
        return th_client_acceptor_start_tcp(client_acceptor);
    }
}

TH_PRIVATE(void)
th_client_acceptor_stop(th_client_acceptor* client_acceptor)
{
    client_acceptor->running = 0;
    th_acceptor_cancel(client_acceptor->acceptor);
    th_client_tracker_cancel_all(&client_acceptor->client_tracker);
}

TH_PRIVATE(void)
th_client_acceptor_deinit(th_client_acceptor* client_acceptor)
{
    th_client_tracker_deinit(&client_acceptor->client_tracker);
#if TH_WITH_SSL
    if (client_acceptor->ssl_enabled) {
        th_ssl_context_deinit(&client_acceptor->ssl_context);
    }
#endif /* TH_WITH_SSL */
}
