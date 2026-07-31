#include "th_conn.h"

/* th_conn_observable begin */

TH_PRIVATE(void)
th_conn_observable_destroy(void* self)
{
    th_conn_observable* observable = self;
    th_conn_observer_on_deinit(observable->observer, observable);
    observable->destroy(observable);
}

TH_PRIVATE(void)
th_conn_observable_init(th_conn_observable* observable, const th_conn_methods* methods,
                        void (*destroy)(void* self), th_conn_observer* observer)
{
    /* methods->destroy must already be th_conn_observable_destroy: the
     * concrete conn type's static methods table points destroy there
     * so th_conn_destroy always notifies the observer first, then this
     * calls the type's real destructor (the destroy param below). */
    observable->base.methods = methods;
    th_conn_observer_on_init(observer, observable);
    observable->destroy = destroy;
    observable->observer = observer;
}

/* th_conn_observable end */
