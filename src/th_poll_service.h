#ifndef TH_POLL_SERVICE_H
#define TH_POLL_SERVICE_H

#include <th.h>

#include "th_config.h"

#ifdef TH_CONFIG_WITH_POLL
#include "th_allocator.h"
#include "th_hashmap.h"
#include "th_io_service.h"
#include "th_io_task.h"
#include "th_runner.h"
#include "th_timer.h"
#include "th_vec.h"

#include <poll.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

/* Forward declarations begin */

typedef struct th_poll_service th_poll_service;
typedef struct th_poll_handle th_poll_handle;
typedef struct th_poll_handle_map th_poll_handle_map;

/* Forward declarations end */
/* th_fd_to_idx_map implementation begin */

TH_INLINE(uint32_t)
th_fd_hash(int fd)
{
    return (uint32_t)fd;
}

TH_INLINE(bool)
th_int_eq(int a, int b)
{
    return a == b;
}

TH_DEFINE_HASHMAP(th_fd_to_idx_map, int, size_t, th_fd_hash, th_int_eq, -1)

/* th_fd_to_idx_map implementation end */
/* th_poll_handle begin */

struct th_poll_handle {
    th_io_handle base;
    th_timer timer;
    th_poll_handle* next;
    th_poll_handle* prev;
    th_allocator* allocator;
    th_poll_service* service;
    th_io_task* iot[TH_IO_OP_TYPE_MAX];
    int fd;
    bool timeout_enabled;
};

#ifndef TH_POLL_HANDLE_POOL
#define TH_POLL_HANDLE_POOL
TH_DEFINE_OBJ_POOL_ALLOCATOR(th_poll_handle_pool, th_poll_handle, prev, next)
#endif

#ifndef TH_POLL_HANDLE_LIST
#define TH_POLL_HANDLE_LIST
TH_DEFINE_QUEUE(th_poll_handle_list, th_poll_handle)
#endif

#ifndef TH_POLLFD_VEC
#define TH_POLLFD_VEC
TH_DEFINE_VEC(th_pollfd_vec, struct pollfd, (void))
#endif

/* th_poll_handle end */
/* th_poll_handle_map begin */

struct th_poll_handle_map {
    th_fd_to_idx_map fd_to_idx_map;
    th_allocator* allocator;
    th_poll_handle** handles;
    size_t size;
    size_t capacity;
};

/* th_poll_handle_map end */

struct th_poll_service {
    th_io_service base;
    th_allocator* allocator;
    th_runner* runner;
    th_poll_handle_pool handle_allocator;
    th_poll_handle_map handles;
    th_pollfd_vec fds;
};

TH_PRIVATE(th_err)
th_poll_service_create(th_io_service** out, th_runner* runner, th_allocator* allocator);

#endif /* TH_HAVE_POLL */
#endif
