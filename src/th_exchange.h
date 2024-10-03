#ifndef TH_EXCHANGE
#define TH_EXCHANGE

#include "th_io_composite.h"
#include "th_request.h"
#include "th_router.h"
#include "th_socket.h"

#define TH_EXCHANGE_CONTINUE (size_t)0
#define TH_EXCHANGE_CLOSE (size_t)1

typedef enum th_request_read_mode {
    TH_REQUEST_READ_MODE_NORMAL = 0,
    TH_REQUEST_READ_MODE_REJECT_UNAVAILABLE = (int)TH_ERR_HTTP(TH_CODE_SERVICE_UNAVAILABLE),
    TH_REQUEST_READ_MODE_REJECT_TOO_MANY_REQUESTS = (int)TH_ERR_HTTP(TH_CODE_TOO_MANY_REQUESTS),
} th_request_read_mode;

typedef struct th_exchange th_exchange;

TH_PRIVATE(th_err)
th_exchange_create(th_exchange** exchange, th_socket* socket,
                   th_router* router, th_fcache* fcache,
                   th_allocator* allocator, th_io_handler* on_complete);

TH_PRIVATE(void)
th_exchange_start(th_exchange* exchange, th_request_read_mode mode);

#endif
