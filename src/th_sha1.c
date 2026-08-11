#include "th_sha1.h"

#include <stdint.h>
#include <string.h>

TH_LOCAL(uint32_t)
th_sha1_rotl(uint32_t x, int n)
{
    return (x << n) | (x >> (32 - n));
}

TH_LOCAL(void)
th_sha1_process_block(uint32_t state[5], const unsigned char block[64])
{
    uint32_t w[80];
    for (int i = 0; i < 16; ++i) {
        w[i] = ((uint32_t)block[i * 4] << 24) | ((uint32_t)block[i * 4 + 1] << 16)
               | ((uint32_t)block[i * 4 + 2] << 8) | (uint32_t)block[i * 4 + 3];
    }
    for (int i = 16; i < 80; ++i) {
        w[i] = th_sha1_rotl(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
    }

    uint32_t a = state[0], b = state[1], c = state[2], d = state[3], e = state[4];
    for (int i = 0; i < 80; ++i) {
        uint32_t f, k;
        if (i < 20) {
            f = (b & c) | (~b & d);
            k = 0x5A827999u;
        } else if (i < 40) {
            f = b ^ c ^ d;
            k = 0x6ED9EBA1u;
        } else if (i < 60) {
            f = (b & c) | (b & d) | (c & d);
            k = 0x8F1BBCDCu;
        } else {
            f = b ^ c ^ d;
            k = 0xCA62C1D6u;
        }
        uint32_t temp = th_sha1_rotl(a, 5) + f + e + k + w[i];
        e = d;
        d = c;
        c = th_sha1_rotl(b, 30);
        b = a;
        a = temp;
    }

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
}

TH_PRIVATE(void)
th_sha1(th_buffer data, unsigned char digest[TH_SHA1_DIGEST_LEN])
{
    uint32_t state[5] = {0x67452301u, 0xEFCDAB89u, 0x98BADCFEu, 0x10325476u, 0xC3D2E1F0u};
    const unsigned char* bytes = (const unsigned char*)data.ptr;
    size_t len = data.len;
    size_t full_blocks = len / 64;
    for (size_t i = 0; i < full_blocks; ++i) {
        th_sha1_process_block(state, bytes + i * 64);
    }

    unsigned char tail[128] = {0};
    size_t tail_len = len - full_blocks * 64;
    memcpy(tail, bytes + full_blocks * 64, tail_len);
    tail[tail_len] = 0x80;
    size_t padded_len = tail_len < 56 ? 64 : 128;
    uint64_t bit_len = (uint64_t)len * 8;
    for (size_t i = 0; i < 8; ++i) {
        tail[padded_len - 1 - i] = (unsigned char)(bit_len >> (8 * i));
    }
    th_sha1_process_block(state, tail);
    if (padded_len == 128) {
        th_sha1_process_block(state, tail + 64);
    }

    for (int i = 0; i < 5; ++i) {
        digest[i * 4] = (unsigned char)(state[i] >> 24);
        digest[i * 4 + 1] = (unsigned char)(state[i] >> 16);
        digest[i * 4 + 2] = (unsigned char)(state[i] >> 8);
        digest[i * 4 + 3] = (unsigned char)state[i];
    }
}
