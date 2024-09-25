#include <th.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "th_acceptor.h"
#include "th_allocator.h"
#include "th_client.h"
#include "th_listener.h"
#include "th_log.h"

#undef TH_LOG_TAG
#define TH_LOG_TAG "listener"

TH_LOCAL(th_err)
th_listener_init(th_listener* listener, th_context* context,
                 const char* host, const char* port,
                 th_router* router, th_fcache* fcache,
                 th_listener_opt* opt, th_allocator* allocator)
{
    listener->allocator = allocator;
    listener->router = router;
    listener->fcache = fcache;
    th_err err = TH_ERR_OK;
    if ((err = th_acceptor_init(&listener->acceptor, context, allocator, host, port)) != TH_ERR_OK)
        goto cleanup;
    if ((err = th_client_acceptor_init(&listener->client_acceptor,
                                       context, listener->router, listener->fcache,
                                       &listener->acceptor, allocator))
        != TH_ERR_OK)
        goto cleanup_acceptor;
    if (opt && opt->key_file && opt->cert_file) {
        if ((err = th_client_acceptor_enable_ssl(&listener->client_acceptor, opt->key_file, opt->cert_file)) != TH_ERR_OK) {
            goto cleanup_client_acceptor;
        }
    }
    TH_LOG_INFO("Created listener on %s:%s", host, port);
    return TH_ERR_OK;
cleanup_client_acceptor:
    th_client_acceptor_deinit(&listener->client_acceptor);
cleanup_acceptor:
    th_acceptor_deinit(&listener->acceptor);
cleanup:
    return err;
}

TH_PRIVATE(th_err)
th_listener_create(th_listener** out, th_context* context,
                   const char* host, const char* port,
                   th_router* router, th_fcache* fcache,
                   th_listener_opt* opt, th_allocator* allocator)
{
    th_listener* listener = th_allocator_alloc(allocator, sizeof(th_listener));
    if (!listener)
        return TH_ERR_BAD_ALLOC;
    th_err err = TH_ERR_OK;
    if ((err = th_listener_init(listener, context, host, port, router, fcache, opt, allocator)) != TH_ERR_OK)
        goto cleanup;
    *out = listener;
    return TH_ERR_OK;
cleanup:
    th_allocator_free(allocator, listener);
    return err;
}

TH_PRIVATE(th_err)
th_listener_start(th_listener* listener)
{
    return th_client_acceptor_start(&listener->client_acceptor);
}

TH_PRIVATE(void)
th_listener_stop(th_listener* listener)
{
    th_client_acceptor_stop(&listener->client_acceptor);
}

TH_LOCAL(void)
th_listener_deinit(th_listener* listener)
{
    th_acceptor_deinit(&listener->acceptor);
    th_client_acceptor_deinit(&listener->client_acceptor);
}

TH_PRIVATE(void)
th_listener_destroy(th_listener* listener)
{
    th_listener_deinit(listener);
    th_allocator_free(listener->allocator, listener);
}
