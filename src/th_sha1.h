#ifndef TH_SHA1_H
#define TH_SHA1_H

#include <th.h>

#include "th_config.h"

#define TH_SHA1_DIGEST_LEN 20

/** th_sha1
 * @brief Computes the SHA-1 digest of data into digest[TH_SHA1_DIGEST_LEN].
 */
TH_PRIVATE(void)
th_sha1(th_buffer data, unsigned char digest[TH_SHA1_DIGEST_LEN]);

#endif
