#include "th_io_task.h"
#include "th_mock_service.h"
#include "th_test.h"

TH_TEST_BEGIN(io_task)
{
    TH_TEST_CASE_BEGIN(io_task_create_and_destroy)
    {
        th_io_task* iot = th_io_task_create(th_default_allocator_get());
        TH_EXPECT(iot != NULL);
        TH_EXPECT(iot->base.destroy != NULL);
        th_io_task_destroy(iot);
    }
    TH_TEST_CASE_END
}
TH_TEST_END
