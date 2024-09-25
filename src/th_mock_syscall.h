#ifndef TH_MOCK_FD_H
#define TH_MOCK_FD_H

#include <stddef.h>

#include "th_iov.h"
#include "th_system_error.h"

typedef struct th_mock_syscall {
    int (*accept)(void);
    int (*open)(void);
    int (*lseek)(void);
    int (*close)(void);
    int (*read)(void* buf, size_t len);
    int (*write)(size_t len);
    int (*settime)(void);
} th_mock_syscall;

th_mock_syscall* th_mock_syscall_get(void);
void th_mock_syscall_reset(void);

int th_mock_accept(void);

int th_mock_open(void);

int th_mock_lseek(void);

int th_mock_close(void);

int th_mock_read(void* buf, size_t len);

int th_mock_write(size_t len);

int th_mock_settime(void);

#endif
