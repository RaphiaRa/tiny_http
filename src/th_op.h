#ifndef TH_OP_H
#define TH_OP_H

#include <th.h>

#include "th_task.h"

#include <stdint.h>

/** th_op_type
 * @brief Which readiness an op is waiting for.
 */
typedef enum th_op_type {
    TH_OP_READ = 0,
    TH_OP_WRITE = 1,
    TH_OP_MAX = 2,
} th_op_type;

/** th_op_flags
 * @brief TH_OP_COMPLETED marks that op->base.fn should finalize (e.g.
 * invoke a user callback) rather than perform I/O again; it is set right
 * before the op is posted to a th_loop, so finalization always runs from
 * a queue drain rather than synchronously inside the call that completed
 * the I/O — this bounds stack depth when I/O completes immediately over
 * and over (e.g. a fast local socket).
 *
 * TH_OP_IMMEDIATE marks that this op has not yet had its first real
 * attempt (set once at th_op_init). th_handle_submit checks it before
 * calling th_op_perform: if set, it tries the op inline right now — this
 * lets an op that's immediately satisfiable (e.g. data already buffered)
 * complete without ever touching the reactor. An op's perform function
 * must clear TH_OP_IMMEDIATE unconditionally on its very first attempt,
 * before checking the result — so on EAGAIN/EWOULDBLOCK it is already
 * clear on all resubmissions from then on. Once clear, th_handle_submit
 * skips the inline attempt entirely and registers straight for real
 * readiness. Without this, a submit that hits EAGAIN would recurse into
 * th_handle_submit -> th_op_perform -> the op's fn -> submit again, once
 * per retry with no real event ever separating attempts (e.g. a
 * listening socket with no pending connection loops until the stack
 * overflows, since nothing ever changes between synchronous attempts).
 */
typedef uint32_t th_op_flags;
#define TH_OP_COMPLETED ((th_op_flags)1 << 0)
#define TH_OP_IMMEDIATE ((th_op_flags)1 << 1)

/** th_op
 * @brief A task submitted to a th_handle (see th_reactor.h). th_handle_submit
 * runs op->base.fn immediately; on TH_EAGAIN/TH_EWOULDBLOCK it registers the
 * op for readiness and fn runs again once ready. On timeout/cancellation/
 * error it calls abort instead (with a th_err describing why), and fn is
 * never invoked for that attempt.
 */
typedef struct th_op {
    th_task base;
    void (*abort)(void* self, th_err err);
    th_op_type type;
    th_op_flags flags;
} th_op;

TH_INLINE(void)
th_op_init(th_op* op, th_op_type type, void (*fn)(void* self), void (*destroy)(void* self), void (*abort)(void* self, th_err err))
{
    th_task_init(&op->base, fn, destroy);
    op->abort = abort;
    op->type = type;
    op->flags = TH_OP_IMMEDIATE;
}

/** th_op_perform
 * @brief Runs the op's fn: performs I/O if not yet TH_OP_COMPLETED, or
 * finalizes (e.g. invokes a user callback) if it is.
 */
TH_INLINE(void)
th_op_perform(th_op* op)
{
    th_task_complete(&op->base);
}

TH_INLINE(void)
th_op_abort(th_op* op, th_err err)
{
    op->abort(op, err);
}

TH_INLINE(void)
th_op_set_flags(th_op* op, th_op_flags flags)
{
    op->flags |= flags;
}

TH_INLINE(void)
th_op_clear_flags(th_op* op, th_op_flags flags)
{
    op->flags &= ~flags;
}

TH_INLINE(th_op_flags)
th_op_get_flags(const th_op* op)
{
    return op->flags;
}

#endif
