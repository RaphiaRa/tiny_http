#include "th_url_decode.h"

TH_LOCAL(th_err)
th_url_decode_next(th_str str, size_t* pos, char* out, th_url_decode_type type)
{
    size_t i = *pos;
    if (str.ptr[i] == '%') {
        int c = 0;
        for (size_t k = 0; k < 2; k++) {
            if (i + 1 + k >= str.len)
                return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
            c <<= 4;
            if (str.ptr[i + 1 + k] >= '0' && str.ptr[i + 1 + k] <= '9') {
                c |= str.ptr[i + 1 + k] - '0';
            } else if (str.ptr[i + 1 + k] >= 'a' && str.ptr[i + 1 + k] <= 'f') {
                c |= str.ptr[i + 1 + k] - 'a' + 10;
            } else if (str.ptr[i + 1 + k] >= 'A' && str.ptr[i + 1 + k] <= 'F') {
                c |= str.ptr[i + 1 + k] - 'A' + 10;
            } else {
                return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
            }
        }
        *out = (char)c;
        i += 3;
    } else if (type == TH_URL_DECODE_TYPE_QUERY && str.ptr[i] == '+') {
        *out = ' ';
        i++;
    } else {
        *out = str.ptr[i++];
    }
    *pos = i;
    return TH_ERR_OK;
}

TH_LOCAL(size_t)
th_url_decode_literal_run(th_str input, size_t pos, th_url_decode_type type)
{
    size_t start = pos;
    while (pos < input.len && input.ptr[pos] != '%'
           && !(type == TH_URL_DECODE_TYPE_QUERY && input.ptr[pos] == '+'))
        pos++;
    return pos - start;
}

TH_PRIVATE(th_err)
th_url_decode_string(th_str input, th_string* output, th_url_decode_type type)
{
    th_string_clear(output);

    th_err err = TH_ERR_OK;
    if (input.len == 0)
        return TH_ERR_OK;
    size_t i = 0;
    while (i < input.len) {
        size_t run = th_url_decode_literal_run(input, i, type);
        if (run > 0) {
            if ((err = th_string_append(output, th_str_substr(input, i, run))) != TH_ERR_OK)
                return err;
            i += run;
            continue;
        }
        char c;
        if ((err = th_url_decode_next(input, &i, &c, type)) != TH_ERR_OK) {
            return err;
        }
        if ((err = th_string_push_back(output, c)) != TH_ERR_OK) {
            return err;
        }
    }
    return TH_ERR_OK;
}
