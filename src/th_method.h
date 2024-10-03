#ifndef TH_METHOD_H
#define TH_METHOD_H

#include <th.h>

struct th_method_mapping {
    const char* name;
    th_method method;
};

struct th_method_mapping* th_method_mapping_find(const char* str, size_t len);

#endif
