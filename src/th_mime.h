#ifndef TH_MIME_H
#define TH_MIME_H

#include <th.h>

#include "th_string.h"

struct th_mime_mapping {
    const char* name;
    th_string mime;
};

struct th_mime_mapping* th_mime_mapping_find(const char* ext, size_t len);

#endif
