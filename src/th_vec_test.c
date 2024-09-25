#include "th_test.h"
#include "th_vec.h"

TH_DEFINE_VEC(th_vec_cstr, const char*, (void))

TH_TEST_BEGIN(th_vec)
{
    TH_TEST_CASE_BEGIN(th_vec_init)
    {
        th_vec_cstr vec;
        th_vec_cstr_init(&vec, NULL);
        TH_EXPECT(vec.data == NULL);
        TH_EXPECT(vec.size == 0);
        TH_EXPECT(vec.capacity == 0);
        th_vec_cstr_deinit(&vec);
    }
    TH_TEST_CASE_END
}
TH_TEST_END
