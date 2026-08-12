#include "th_address.h"

#if defined(TH_CONFIG_OS_POSIX)
#include <errno.h>
#include <string.h>

TH_LOCAL(th_err)
th_addrinfo_ops_os_getaddrinfo(void* self, const char* host, const char* port, const struct addrinfo* hints, struct addrinfo** res)
{
    (void)self;
    int rc = getaddrinfo(host, port, hints, res);
    if (rc == 0)
        return TH_ERR_OK;
    return rc == EAI_SYSTEM ? TH_ERR_SYSTEM(errno) : TH_ERR_EAI(rc);
}

TH_LOCAL(void)
th_addrinfo_ops_os_freeaddrinfo(void* self, struct addrinfo* res)
{
    (void)self;
    freeaddrinfo(res);
}

TH_PRIVATE(th_addrinfo_ops*)
th_addrinfo_ops_os(void)
{
    static th_addrinfo_ops ops = {
        .getaddrinfo = th_addrinfo_ops_os_getaddrinfo,
        .freeaddrinfo = th_addrinfo_ops_os_freeaddrinfo,
    };
    return &ops;
}

TH_PRIVATE(const char*)
th_addrinfo_strerror(int code)
{
    return gai_strerror(code);
}

#endif /* TH_CONFIG_OS_POSIX */

TH_PRIVATE(th_err)
th_addrinfo_from_str(th_addrinfo* info, const char* host, const char* port, th_addrinfo_ops* ops)
{
    struct addrinfo hints = {0};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo* res = NULL;
    th_err err = ops->getaddrinfo(ops, host, port, &hints, &res);
    if (err != TH_ERR_OK)
        return err;

    info->family = res->ai_family;
    info->socktype = res->ai_socktype;
    info->protocol = res->ai_protocol;
    info->addr.addrlen = (socklen_t)res->ai_addrlen;
    memcpy(&info->addr.addr, res->ai_addr, res->ai_addrlen);
    ops->freeaddrinfo(ops, res);
    return TH_ERR_OK;
}
