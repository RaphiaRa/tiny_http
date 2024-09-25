#ifndef TH_URL_DECODE_H
#define TH_URL_DECODE_H

#include <th.h>

#include "th_config.h"
#include "th_heap_string.h"

#include <stddef.h>

typedef enum th_url_decode_type {
    TH_URL_DECODE_TYPE_PATH = 0,
    TH_URL_DECODE_TYPE_QUERY
} th_url_decode_type;

/*
TH_PRIVATE(th_err)
th_url_decode_inplace(char* str, size_t* in_out_len, th_url_decode_type type);
*/

TH_PRIVATE(th_err)
th_url_decode_string(th_string input, th_heap_string* output, th_url_decode_type type);

#endif
