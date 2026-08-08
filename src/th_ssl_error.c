#include "th_ssl_error.h"

#if TH_WITH_SSL

#include <openssl/err.h>
#include <openssl/ssl.h>

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

#endif // TH_WITH_SSL
