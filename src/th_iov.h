#ifndef TH_IOV_H
#define TH_IOV_H

#include <stddef.h>
#include <sys/uio.h>

#include "th_config.h"

/** th_iov
 *@brief I/O vector.
 */

typedef struct th_iov {
    void* base;
    size_t len;
} th_iov;

/** th_iov_consume
 *@brief Consume the I/O vector and
 * return the number of bytes that were not consumed.
 */
TH_INLINE(size_t)
th_iov_consume(th_iov** iov, size_t* iov_len, size_t consume)
{
    size_t zeroed = 0;
    for (size_t i = 0; i < *iov_len; i++) {
        if (consume < (*iov)[i].len) {
            (*iov)[i].base = (char*)(*iov)[i].base + consume;
            (*iov)[i].len -= consume;
            consume = 0;
            break;
        }
        consume -= (*iov)[i].len;
        (*iov)[i].len = 0;
        zeroed++;
    }
    *iov_len -= zeroed;
    (*iov) += zeroed;
    return consume;
}

TH_INLINE(size_t)
th_iov_bytes(th_iov* iov, size_t iov_len)
{
    size_t bytes = 0;
    for (size_t i = 0; i < iov_len; i++) {
        bytes += iov[i].len;
    }
    return bytes;
}

#endif
