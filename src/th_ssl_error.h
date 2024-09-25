#ifndef TH_SSL_ERROR_H
#define TH_SSL_ERROR_H

#if TH_WITH_SSL
#include <th.h>

#include "th_config.h"

TH_PRIVATE(void)
th_ssl_log_error_stack(void);

TH_PRIVATE(const char*)
th_ssl_strerror(int code);

TH_PRIVATE(th_err)
th_ssl_handle_error_stack(void);

#endif // TH_WITH_SSL
#endif
