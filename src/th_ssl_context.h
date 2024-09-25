#ifndef TH_SSL_CONTEXT_H
#define TH_SSL_CONTEXT_H

#include "th_config.h"

#if TH_WITH_SSL
#include <th.h>

#include <openssl/ssl.h>

typedef struct th_ssl_context {
    SSL_CTX* ctx;
    BIO_METHOD* smem_method;
} th_ssl_context;

TH_PRIVATE(th_err)
th_ssl_context_init(th_ssl_context* context, const char* key, const char* cert);

TH_PRIVATE(void)
th_ssl_context_deinit(th_ssl_context* context);

#endif
#endif
