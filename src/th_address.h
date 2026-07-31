#ifndef TH_ADDRESS_H
#define TH_ADDRESS_H

#include <th.h>

#include "th_config.h"

#include <sys/socket.h>

/** th_address
 * @brief Storage for a peer address filled in by th_acceptor_ops.accept.
 */
typedef struct th_address {
    struct sockaddr_storage addr;
    socklen_t addrlen;
} th_address;

TH_INLINE(void)
th_address_init(th_address* addr)
{
    addr->addrlen = sizeof(addr->addr);
}

#endif
