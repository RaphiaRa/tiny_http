#include "th_conn.h"
#include "th_test.h"

typedef struct th_fake_conn {
    th_conn_observable base;
    bool destroyed;
} th_fake_conn;

static th_address*
th_fake_conn_get_address(void* self)
{
    (void)self;
    return NULL;
}

static void
th_fake_conn_start(void* self)
{
    (void)self;
}

static void
th_fake_conn_cancel(void* self)
{
    (void)self;
}

static void
th_fake_conn_free(void* self)
{
    th_fake_conn* conn = self;
    conn->destroyed = true;
}

static const th_conn_methods th_fake_conn_methods = {
    .get_address = th_fake_conn_get_address,
    .start = th_fake_conn_start,
    .recv = NULL,
    .send = NULL,
    .cancel = th_fake_conn_cancel,
    .destroy = th_conn_observable_destroy,
};

typedef struct th_recording_observer {
    th_conn_observer base;
    int init_count;
    int deinit_count;
    th_conn_observable* last_observable;
} th_recording_observer;

static void
th_recording_observer_on_init(th_conn_observer* self, th_conn_observable* observable)
{
    th_recording_observer* observer = (th_recording_observer*)self;
    ++observer->init_count;
    observer->last_observable = observable;
}

static void
th_recording_observer_on_deinit(th_conn_observer* self, th_conn_observable* observable)
{
    th_recording_observer* observer = (th_recording_observer*)self;
    ++observer->deinit_count;
    observer->last_observable = observable;
}

static void
th_recording_observer_init(th_recording_observer* observer)
{
    observer->base.on_init = th_recording_observer_on_init;
    observer->base.on_deinit = th_recording_observer_on_deinit;
    observer->init_count = 0;
    observer->deinit_count = 0;
    observer->last_observable = NULL;
}

static th_conn** th_recording_upgrade_target;

static void
th_recording_upgrade_fn(void* self, th_conn* conn)
{
    (void)self;
    *th_recording_upgrade_target = conn;
}

TH_TEST_BEGIN(conn)
{
    TH_TEST_CASE_BEGIN(conn_observable_init_notifies_observer)
    {
        th_recording_observer observer;
        th_recording_observer_init(&observer);
        th_fake_conn conn;
        conn.destroyed = false;

        th_conn_observable_init(&conn.base, &th_fake_conn_methods, th_fake_conn_free, &observer.base);

        TH_EXPECT(observer.init_count == 1);
        TH_EXPECT(observer.last_observable == &conn.base);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(conn_destroy_notifies_observer_then_frees)
    {
        th_recording_observer observer;
        th_recording_observer_init(&observer);
        th_fake_conn conn;
        conn.destroyed = false;
        th_conn_observable_init(&conn.base, &th_fake_conn_methods, th_fake_conn_free, &observer.base);

        th_conn_destroy(&conn.base.base);

        TH_EXPECT(observer.deinit_count == 1);
        TH_EXPECT(conn.destroyed);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(conn_upgrader_forwards_to_upgrade_fn)
    {
        th_recording_observer observer;
        th_recording_observer_init(&observer);
        th_fake_conn conn;
        conn.destroyed = false;
        th_conn_observable_init(&conn.base, &th_fake_conn_methods, th_fake_conn_free, &observer.base);

        th_conn* upgraded = NULL;
        th_conn_upgrader upgrader;
        th_conn_upgrader_init(&upgrader, th_recording_upgrade_fn);
        th_recording_upgrade_target = &upgraded;
        th_conn_upgrader_upgrade(&upgrader, &conn.base.base);

        TH_EXPECT(upgraded == &conn.base.base);
    }
    TH_TEST_CASE_END
}
TH_TEST_END
