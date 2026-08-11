#ifndef TH_WS_FRAME_H
#define TH_WS_FRAME_H

#include <th.h>

#include "th_config.h"

typedef enum th_ws_frame_type {
    TH_WS_FRAME_DATA,
    TH_WS_FRAME_PING,
    TH_WS_FRAME_PONG,
    TH_WS_FRAME_CLOSE,
} th_ws_frame_type;

#endif
