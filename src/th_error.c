#include <string.h>
#include <th.h>

#include "th_address.h"
#include "th_http_error.h"
#include "th_ssl_error.h"
#include "th_utility.h"

TH_PUBLIC(const char*)
th_strerror(th_err err)
{
    switch (TH_ERR_CATEGORY(err)) {
    case TH_ERR_CATEGORY_OTHER:
        switch (TH_ERR_CODE(err)) {
        case 0:
            return "success";
        case TH_ERRC_BAD_ALLOC:
            return "out of memory";
        case TH_ERRC_INVALID_ARG:
            return "invalid argument";
        case TH_ERRC_EOF:
            return "end of file";
        case TH_ERRC_BUSY:
            return "busy";
        default:
            return "unknown error";
        }
        break;
    case TH_ERR_CATEGORY_SYSTEM:
        return strerror(TH_ERR_CODE(err));
    case TH_ERR_CATEGORY_HTTP:
        return th_http_strerror(TH_ERR_CODE(err));
    case TH_ERR_CATEGORY_SSL:
#if TH_WITH_SSL
        return th_ssl_strerror(TH_ERR_CODE(err));
#else
        TH_ASSERT(0 && "SSL not enabled");
        return NULL;
#endif
    case TH_ERR_CATEGORY_EAI:
        return th_addrinfo_strerror(TH_ERR_CODE(err));
    default:
        break;
    }
    return "Unknown error category";
}
