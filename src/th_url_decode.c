#include "th_url_decode.h"

TH_LOCAL(th_err)
th_url_decode_next(const char* str, size_t* pos, char* out, th_url_decode_type type)
{
    size_t i = *pos;
    if (str[i] == '%') {
        char c = 0;
        for (size_t k = 0; k < 2; k++) {
            c <<= 4;
            if (str[i + 1 + k] >= '0' && str[i + 1 + k] <= '9') {
                c |= str[i + 1 + k] - '0';
            } else if (str[i + 1 + k] >= 'a' && str[i + 1 + k] <= 'f') {
                c |= str[i + 1 + k] - 'a' + 10;
            } else if (str[i + 1 + k] >= 'A' && str[i + 1 + k] <= 'F') {
                c |= str[i + 1 + k] - 'A' + 10;
            } else {
                return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
            }
        }
        *out = c;
        i += 3;
    } else if (type == TH_URL_DECODE_TYPE_QUERY && str[i] == '+') {
        *out = ' ';
        i++;
    } else {
        *out = str[i++];
    }
    *pos = i;
    return TH_ERR_OK;
}

/*
TH_PRIVATE(th_err)
th_url_decode_inplace(char* str, size_t* in_out_len, th_url_decode_type type)
{
    size_t i = 0;
    size_t j = 0;
    size_t len = *in_out_len;
    while (i < len) {
        char c;
        th_err err = th_url_decode_next(str, &i, &c, type);
        if (err != TH_ERR_OK) {
            return err;
        }
        str[j++] = c;
    }
    str[j] = '\0';
    *in_out_len = j;
    return TH_ERR_OK;
}
*/

TH_PRIVATE(th_err)
th_url_decode_string(th_string input, th_heap_string* output, th_url_decode_type type)
{
    th_heap_string_clear(output);

    th_err err = TH_ERR_OK;
    size_t len = input.len;
    if (len == 0)
        return TH_ERR_OK;
    size_t i = 0;
    while (i < len) {
        char c;
        if ((err = th_url_decode_next(input.ptr, &i, &c, type)) != TH_ERR_OK) {
            return err;
        }
        if ((err = th_heap_string_push_back(output, c)) != TH_ERR_OK) {
            return err;
        }
    }
    return TH_ERR_OK;
}
