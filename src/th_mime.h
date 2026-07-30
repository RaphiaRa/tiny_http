#ifndef TH_MIME_H
#define TH_MIME_H

#include <th.h>

#include "th_str.h"

struct th_mime_mapping {
    const char* name;
    th_str mime;
};

struct th_mime_mapping* th_mime_mapping_find(const char* ext, size_t len);

#endif
