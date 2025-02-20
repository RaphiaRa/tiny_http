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
    if (err == TH_ERR_OK)
        return TH_ERR_HTTP(TH_CODE_OK);
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
    case TH_CODE_INTERNAL_SERVER_ERROR:
        return "Internal Server Error";
        break;
    case TH_CODE_SERVICE_UNAVAILABLE:
        return "Service Unavailable";
        break;
    case TH_CODE_NOT_IMPLEMENTED:
        return "Method Not Implemented";
        break;
    case TH_CODE_REQUEST_TIMEOUT:
        return "Request Timeout";
        break;
    case TH_CODE_TOO_MANY_REQUESTS:
        return "Too Many Requests";
        break;
    case TH_CODE_URI_TOO_LONG:
        return "URI Too Long";
        break;
    case TH_CODE_UNSUPPORTED_MEDIA_TYPE:
        return "Unsupported Media Type";
        break;
    case TH_CODE_RANGE_NOT_SATISFIABLE:
        return "Range Not Satisfiable";
        break;
    case TH_CODE_REQUEST_HEADER_FIELDS_TOO_LARGE:
        return "Request Header Fields Too Large";
        break;
    case TH_CODE_UNAUTHORIZED:
        return "Unauthorized";
        break;
    case TH_CODE_FORBIDDEN:
        return "Forbidden";
        break;
    default:
        return "Unknown";
        break;
    }
}

typedef enum th_http_code_type {
    TH_HTTP_CODE_TYPE_INFORMATIONAL,
    TH_HTTP_CODE_TYPE_SUCCESS,
    TH_HTTP_CODE_TYPE_REDIRECT,
    TH_HTTP_CODE_TYPE_CLIENT_ERROR,
    TH_HTTP_CODE_TYPE_SERVER_ERROR,
} th_http_code_type;

TH_INLINE(th_http_code_type)
th_http_code_get_type(int code)
{
    if (code >= 100 && code < 200)
        return TH_HTTP_CODE_TYPE_INFORMATIONAL;
    if (code >= 200 && code < 300)
        return TH_HTTP_CODE_TYPE_SUCCESS;
    if (code >= 300 && code < 400)
        return TH_HTTP_CODE_TYPE_REDIRECT;
    if (code >= 400 && code < 500)
        return TH_HTTP_CODE_TYPE_CLIENT_ERROR;
    if (code >= 500 && code < 600)
        return TH_HTTP_CODE_TYPE_SERVER_ERROR;
    return TH_HTTP_CODE_TYPE_SERVER_ERROR;
}

#endif
