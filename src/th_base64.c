#include "th_base64.h"

#include <stdint.h>

static const char th_base64_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

TH_LOCAL(size_t)
th_base64_encoded_len(size_t len)
{
    return ((len + 2) / 3) * 4;
}

TH_PRIVATE(th_err)
th_base64_encode(th_str input, th_string* output)
{
    th_err err = th_string_resize(output, th_base64_encoded_len(input.len), '\0');
    if (err != TH_ERR_OK)
        return err;
    if (input.len == 0)
        return TH_ERR_OK;

    const unsigned char* bytes = (const unsigned char*)input.ptr;
    char* out = th_string_at(output, 0);
    size_t i = 0;
    size_t o = 0;
    for (; i + 3 <= input.len; i += 3) {
        uint32_t n = ((uint32_t)bytes[i] << 16) | ((uint32_t)bytes[i + 1] << 8) | (uint32_t)bytes[i + 2];
        out[o++] = th_base64_alphabet[(n >> 18) & 0x3F];
        out[o++] = th_base64_alphabet[(n >> 12) & 0x3F];
        out[o++] = th_base64_alphabet[(n >> 6) & 0x3F];
        out[o++] = th_base64_alphabet[n & 0x3F];
    }
    size_t remaining = input.len - i;
    if (remaining == 1) {
        uint32_t n = (uint32_t)bytes[i] << 16;
        out[o++] = th_base64_alphabet[(n >> 18) & 0x3F];
        out[o++] = th_base64_alphabet[(n >> 12) & 0x3F];
        out[o++] = '=';
        out[o++] = '=';
    } else if (remaining == 2) {
        uint32_t n = ((uint32_t)bytes[i] << 16) | ((uint32_t)bytes[i + 1] << 8);
        out[o++] = th_base64_alphabet[(n >> 18) & 0x3F];
        out[o++] = th_base64_alphabet[(n >> 12) & 0x3F];
        out[o++] = th_base64_alphabet[(n >> 6) & 0x3F];
        out[o++] = '=';
    }
    return TH_ERR_OK;
}
