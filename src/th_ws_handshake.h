#ifndef TH_WS_HANDSHAKE_H
#define TH_WS_HANDSHAKE_H

#include <th.h>

#include "th_config.h"
#include "th_request.h"
#include "th_str.h"
#include "th_string.h"

/** th_ws_is_handshake
 * @brief Checks whether request is a valid RFC 6455 upgrade request.
 */
TH_PRIVATE(bool)
th_ws_is_handshake(th_request* request);

/** th_ws_handshake_accept_key
 * @brief Computes Sec-WebSocket-Accept for a Sec-WebSocket-Key value.
 * @return TH_ERR_INVALID_ARG if key is too long, TH_ERR_OK otherwise.
 */
TH_PRIVATE(th_err)
th_ws_handshake_accept_key(th_str key, th_string* out);

#endif
