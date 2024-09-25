#ifndef TH_MOCK_SERVICE_H
#define TH_MOCK_SERVICE_H

#include <th.h>

#include "th_config.h"

#if defined(TH_CONFIG_OS_MOCK)

#include "th_io_service.h"
#include "th_io_task.h"
#include "th_runner.h"

typedef struct th_mock_service th_mock_service;
typedef struct th_mock_handle th_mock_handle;
struct th_mock_handle {
    th_io_handle base;
    th_mock_service* service;
    int fd;
};

struct th_mock_service {
    th_io_service base;
    th_runner* runner;
};

TH_PRIVATE(th_err)
th_mock_service_create(th_io_service** out, th_runner* runner);

#endif
#endif
