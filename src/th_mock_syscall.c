#include "th_mock_syscall.h"

static int th_mock_accept_default(void)
{
    return 0;
}

static int th_mock_open_default(void)
{
    return 0;
}

static int th_mock_lseek_default(void)
{
    return 0;
}

static int th_mock_close_default(void)
{
    return 0;
}

static int th_mock_read_default(void* buf, size_t len)
{
    (void)buf;
    return (int)len;
}

static int th_mock_write_default(size_t len)
{
    return (int)len;
}

static int th_mock_settime_default(void)
{
    return 0;
}

th_mock_syscall* th_mock_syscall_get(void)
{
    static th_mock_syscall syscall = {
        .accept = th_mock_accept_default,
        .open = th_mock_open_default,
        .lseek = th_mock_lseek_default,
        .close = th_mock_close_default,
        .read = th_mock_read_default,
        .write = th_mock_write_default,
        .settime = th_mock_settime_default,
    };
    return &syscall;
}

void th_mock_syscall_reset(void)
{
    th_mock_syscall* syscall = th_mock_syscall_get();
    syscall->accept = th_mock_accept_default;
    syscall->open = th_mock_open_default;
    syscall->lseek = th_mock_lseek_default;
    syscall->close = th_mock_close_default;
    syscall->read = th_mock_read_default;
    syscall->write = th_mock_write_default;
    syscall->settime = th_mock_settime_default;
}

int th_mock_accept(void)
{
    th_mock_syscall* syscall = th_mock_syscall_get();
    return syscall->accept();
}

int th_mock_open(void)
{
    th_mock_syscall* syscall = th_mock_syscall_get();
    return syscall->open();
}

int th_mock_lseek(void)
{
    th_mock_syscall* syscall = th_mock_syscall_get();
    return syscall->lseek();
}

int th_mock_close(void)
{
    th_mock_syscall* syscall = th_mock_syscall_get();
    return syscall->close();
}

int th_mock_read(void* buf, size_t len)
{
    th_mock_syscall* syscall = th_mock_syscall_get();
    return syscall->read(buf, len);
}

int th_mock_write(size_t len)
{
    th_mock_syscall* syscall = th_mock_syscall_get();
    return syscall->write(len);
}

int th_mock_settime(void)
{
    th_mock_syscall* syscall = th_mock_syscall_get();
    return syscall->settime();
}
