#ifndef TH_KQUEUE_SERVICE_H
#define TH_KQUEUE_SERVICE_H

#include <th.h>

#include "th_config.h"

#ifdef TH_CONFIG_WITH_KQUEUE
#include "th_allocator.h"
#include "th_hashmap.h"
#include "th_io_service.h"
#include "th_io_task.h"
#include "th_list.h"
#include "th_runner.h"
#include "th_timer.h"

#include <sys/event.h>
#include <sys/time.h>
#include <sys/types.h>

/* Forward declarations begin */

typedef struct th_kqueue_service th_kqueue_service;
typedef struct th_kqueue_handle th_kqueue_handle;
typedef struct th_kqueue_handle_cleaner th_kqueue_handle_cleaner;

/* Forward declarations end */

struct th_kqueue_handle {
    th_io_handle base;
    th_timer timer;
    th_allocator* allocator;
    th_kqueue_handle* pool_next;
    th_kqueue_handle* pool_prev;
    th_kqueue_handle* timer_next;
    th_kqueue_handle* timer_prev;
    th_kqueue_service* service;
    th_io_task* iot[TH_IO_OP_TYPE_MAX];
    int fd;
    th_io_op_type active;
    bool timeout_enabled;
};

#ifndef TH_KQUEUE_HANDLE_POOL
#define TH_KQUEUE_HANDLE_POOL
TH_DEFINE_OBJ_POOL_ALLOCATOR(th_kqueue_handle_pool, th_kqueue_handle, pool_prev, pool_next)
#endif

#ifndef TH_KQUEUE_HANDLE_TIMER_LIST
#define TH_KQUEUE_HANDLE_TIMER_LIST
TH_DEFINE_LIST(th_kqueue_timer_list, th_kqueue_handle, timer_prev, timer_next)
#endif

struct th_kqueue_service {
    th_io_service base;
    th_allocator* allocator;
    th_runner* runner;
    th_kqueue_handle_pool handle_allocator;
    th_kqueue_timer_list timer_list;
    int kq;
};

TH_PRIVATE(th_err)
th_kqueue_service_create(th_io_service** out, th_runner* runner, th_allocator* allocator);

#endif /* TH_HAVE_KQUEUE */
#endif
