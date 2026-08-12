#include "th_address.h"
#include "th_system_error.h"
#include "th_test.h"

#include <netinet/in.h>
#include <string.h>

typedef struct th_fake_addrinfo_ops {
    th_addrinfo_ops base;
    th_err resolve_err;
    const char* last_host;
    const char* last_port;
    struct sockaddr_in sockaddr;
    struct addrinfo addrinfo;
    bool freed;
} th_fake_addrinfo_ops;

static th_err
th_fake_getaddrinfo(void* self, const char* host, const char* port, const struct addrinfo* hints, struct addrinfo** res)
{
    (void)hints;
    th_fake_addrinfo_ops* ops = self;
    ops->last_host = host;
    ops->last_port = port;
    if (ops->resolve_err != TH_ERR_OK)
        return ops->resolve_err;

    ops->sockaddr = (struct sockaddr_in){.sin_family = AF_INET};
    ops->addrinfo = (struct addrinfo){
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
        .ai_protocol = 0,
        .ai_addrlen = sizeof(ops->sockaddr),
        .ai_addr = (struct sockaddr*)&ops->sockaddr,
    };
    *res = &ops->addrinfo;
    return TH_ERR_OK;
}

static void
th_fake_freeaddrinfo(void* self, struct addrinfo* res)
{
    (void)res;
    th_fake_addrinfo_ops* ops = self;
    ops->freed = true;
}

static void
th_fake_addrinfo_ops_init(th_fake_addrinfo_ops* ops)
{
    ops->base.getaddrinfo = th_fake_getaddrinfo;
    ops->base.freeaddrinfo = th_fake_freeaddrinfo;
    ops->resolve_err = TH_ERR_OK;
    ops->last_host = NULL;
    ops->last_port = NULL;
    ops->freed = false;
}

TH_TEST_BEGIN(address)
{
    th_fake_addrinfo_ops ops;
    th_fake_addrinfo_ops_init(&ops);

    TH_TEST_CASE_BEGIN(addrinfo_from_str_forwards_host_port_to_ops)
    {
        th_addrinfo info;
        TH_EXPECT(th_addrinfo_from_str(&info, "example.com", "8080", &ops.base) == TH_ERR_OK);
        TH_EXPECT(strcmp(ops.last_host, "example.com") == 0);
        TH_EXPECT(strcmp(ops.last_port, "8080") == 0);
        TH_EXPECT(info.family == AF_INET);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(addrinfo_from_str_frees_the_resolved_list)
    {
        th_addrinfo info;
        TH_EXPECT(th_addrinfo_from_str(&info, "example.com", "8080", &ops.base) == TH_ERR_OK);
        TH_EXPECT(ops.freed);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(addrinfo_from_str_propagates_ops_error)
    {
        ops.resolve_err = TH_ERR_SYSTEM(TH_EIO);
        th_addrinfo info;
        TH_EXPECT(th_addrinfo_from_str(&info, "example.com", "8080", &ops.base) == TH_ERR_SYSTEM(TH_EIO));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(addrinfo_from_str_resolves_ipv4_literal)
    {
        th_addrinfo info;
        TH_EXPECT(th_addrinfo_from_str(&info, "127.0.0.1", "8080", th_addrinfo_ops_os()) == TH_ERR_OK);
        TH_EXPECT(info.family == AF_INET);
        TH_EXPECT(info.addr.addrlen == sizeof(struct sockaddr_in));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(addrinfo_from_str_resolves_ipv6_literal)
    {
        th_addrinfo info;
        TH_EXPECT(th_addrinfo_from_str(&info, "::1", "8080", th_addrinfo_ops_os()) == TH_ERR_OK);
        TH_EXPECT(info.family == AF_INET6);
        TH_EXPECT(info.addr.addrlen == sizeof(struct sockaddr_in6));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(addrinfo_from_str_rejects_unresolvable_host)
    {
        th_addrinfo info;
        TH_EXPECT(th_addrinfo_from_str(&info, "this.host.does.not.resolve.invalid", "8080", th_addrinfo_ops_os()) != TH_ERR_OK);
    }
    TH_TEST_CASE_END
}
TH_TEST_END
