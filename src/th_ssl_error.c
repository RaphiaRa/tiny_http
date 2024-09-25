#include "th_ssl_error.h"

#if TH_WITH_SSL

#include "th_log.h"

#include <openssl/err.h>
#include <openssl/ssl.h>

#undef TH_LOG_TAG
#define TH_LOG_TAG "ssl"

TH_PRIVATE(void)
th_ssl_log_error_stack(void)
{
    unsigned long code;
    while ((code = ERR_get_error())) {
        TH_LOG_ERROR("%s", ERR_reason_error_string(code));
    }
}

TH_PRIVATE(const char*)
th_ssl_strerror(int code)
{
    switch (code) {
    case SSL_ERROR_NONE:
        return "Success";
        break;
    case SSL_ERROR_SSL:
        return "SSL library error, enable logging for more details";
        break;
    default:
        break;
    }
    return ERR_reason_error_string((unsigned long)code);
}

TH_PRIVATE(th_err)
th_ssl_handle_error_stack(void)
{
    th_ssl_log_error_stack();
    return TH_ERR_SSL(SSL_ERROR_SSL);
}

#endif // TH_WITH_SSL
