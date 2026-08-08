#ifndef TH_SSL_ERROR_H
#define TH_SSL_ERROR_H

#include "th_config.h"

#if TH_WITH_SSL
#include <th.h>

#include "th_config.h"

TH_PRIVATE(const char*)
th_ssl_strerror(int code);

#endif // TH_WITH_SSL
#endif
