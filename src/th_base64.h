#ifndef TH_BASE64_H
#define TH_BASE64_H

#include "th_config.h"
#include "th_str.h"
#include "th_string.h"

/** th_base64_encode
 * @brief Base64-encodes input, overwriting output with the result.
 */
TH_PRIVATE(th_err)
th_base64_encode(th_str input, th_string* output);

#endif
