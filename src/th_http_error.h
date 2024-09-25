#ifndef TH_HTTP_ERR_H
#define TH_HTTP_ERR_H

#include <th.h>

#include "th_config.h"
#include "th_system_error.h"

#include <errno.h>

/** th_http_err
 * @brief Converts a error code to a equivalent HTTP error code.
 */
TH_INLINE(th_err)
th_http_error(th_err err)
{
    switch (TH_ERR_CATEGORY(err)) {
    case TH_ERR_CATEGORY_SYSTEM:
        switch (TH_ERR_CODE(err)) {
            {
            case TH_ENOENT:
                return TH_ERR_HTTP(TH_CODE_NOT_FOUND);
                break;
            case TH_ETIMEDOUT:
                return TH_ERR_HTTP(TH_CODE_REQUEST_TIMEOUT);
                break;
            default:
                return TH_ERR_HTTP(TH_CODE_INTERNAL_SERVER_ERROR);
                break;
            }
        }
        break;
    case TH_ERR_CATEGORY_HTTP:
        return err;
        break;
    }
    return TH_ERR_HTTP(TH_CODE_INTERNAL_SERVER_ERROR);
}

TH_INLINE(const char*)
th_http_strerror(int code)
{
    switch (code) {
    case TH_CODE_OK:
        return "OK";
        break;
    case TH_CODE_MOVED_PERMANENTLY:
        return "Moved Permanently";
        break;
    case TH_CODE_BAD_REQUEST:
        return "Bad Request";
        break;
    case TH_CODE_NOT_FOUND:
        return "Not Found";
        break;
    case TH_CODE_METHOD_NOT_ALLOWED:
        return "Method Not Allowed";
        break;
    case TH_CODE_PAYLOAD_TOO_LARGE:
        return "Payload Too Large";
        break;
    case TH_CODE_REQUEST_HEADER_TOO_LARGE:
        return "Request Header Too Large";
        break;
    case TH_CODE_INTERNAL_SERVER_ERROR:
        return "Internal Server Error";
        break;
    case TH_CODE_NOT_IMPLEMENTED:
        return "Method Not Implemented";
        break;
    default:
        return "Unknown";
        break;
    }
}

#endif
