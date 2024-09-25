#ifndef TH_UNIQUE_HEADER_ID_H
#define TH_UNIQUE_HEADER_ID_H

#include "th_config.h"

#include <stddef.h>
#include <stdint.h>

typedef enum th_header_id {
    TH_HEADER_ID_CONNECTION,
    TH_HEADER_ID_CONTENT_LENGTH,
    TH_HEADER_ID_CONTENT_TYPE,
    TH_HEADER_ID_DATE,
    TH_HEADER_ID_SERVER,
    TH_HEADER_ID_MAX,
    TH_HEADER_ID_UNKNOWN = TH_HEADER_ID_MAX,
    TH_HEADER_ID_COOKIE,
} th_header_id;

struct th_header_id_mapping {
    const char* name;
    th_header_id id;
};

struct th_header_id_mapping*
th_header_id_mapping_find(const char* name, size_t len);

TH_INLINE(th_header_id)
th_header_id_from_string(const char* name, size_t len)
{
    struct th_header_id_mapping* mapping = th_header_id_mapping_find(name, (unsigned int)len);
    return mapping ? mapping->id : TH_HEADER_ID_UNKNOWN;
}

#endif
