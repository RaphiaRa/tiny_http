#ifndef TH_METHOD_H
#define TH_METHOD_H

#include <th.h>

typedef enum th_method_internal {
    TH_METHOD_INTERNAL_GET = TH_METHOD_GET,
    TH_METHOD_INTERNAL_POST = TH_METHOD_POST,
    TH_METHOD_INTERNAL_PUT = TH_METHOD_PUT,
    TH_METHOD_INTERNAL_DELETE = TH_METHOD_DELETE,
    TH_METHOD_INTERNAL_PATCH = TH_METHOD_PATCH,
    TH_METHOD_INTERNAL_CONNECT,
    TH_METHOD_INTERNAL_OPTIONS,
    TH_METHOD_INTERNAL_TRACE,
    TH_METHOD_INTERNAL_HEAD,
    TH_METHOD_INTERNAL_INVALID
} th_method_internal;

struct th_method_mapping {
    const char* name;
    th_method_internal method;
};

struct th_method_mapping* th_method_mapping_find(const char* str, size_t len);

#endif
