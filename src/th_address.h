#ifndef TH_ADDRESS_H
#define TH_ADDRESS_H

#include <th.h>

#include "th_config.h"

#include <netdb.h>
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

typedef struct th_addrinfo_ops {
    th_err (*getaddrinfo)(void* self, const char* host, const char* port, const struct addrinfo* hints, struct addrinfo** res);
    void (*freeaddrinfo)(void* self, struct addrinfo* res);
} th_addrinfo_ops;

TH_PRIVATE(th_addrinfo_ops*)
th_addrinfo_ops_os(void);

/** th_addrinfo_strerror
 * @brief gai_strerror for an EAI_* code, i.e. TH_ERR_EAI's code - not a
 * POSIX errno, so th_system_strerror doesn't apply.
 */
TH_PRIVATE(const char*)
th_addrinfo_strerror(int code);

typedef struct th_addrinfo {
    th_address addr;
    int family;
    int socktype;
    int protocol;
} th_addrinfo;

TH_PRIVATE(th_err)
th_addrinfo_from_str(th_addrinfo* info, const char* host, const char* port, th_addrinfo_ops* ops);

#endif
