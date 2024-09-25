#ifndef TH_TEST_H
#define TH_TEST_H

#include <stdio.h>

typedef enum {
    TH_TEST_SUCCESS = 0,
    TH_TEST_FAILURE = -1,
} th_test_result;

#define TH_EXPECT(x)                                                   \
    if ((x) == 0) {                                                    \
        printf("Test failed: %s, at %s:%d\n", #x, __FILE__, __LINE__); \
        return TH_TEST_FAILURE;                                        \
    }

/** th_test_setup
 * @brief Setup the test environment. This function is called before any test
 * cases are run. and does the following:
 * - Initializes the test_allocator and sets it as the default allocator.
 */
void th_test_setup(void);

void th_test_teardown(void);

/** th_test_allocator_outstanding
 * @brief Check if there are outstanding allocations.
 * @return The number of outstanding allocations.
 */
int th_test_allocator_outstanding(void);

#define TH_TEST_BEGIN(name)                         \
    int src_th_##name##_test(int argc, char** argv) \
    {                                               \
        (void)argc;                                 \
        (void)argv;

#define TH_TEST_END         \
    return TH_TEST_SUCCESS; \
    }

#define TH_TEST_CASE_BEGIN(name) \
    {                            \
        printf("Running test-case: %40s", #name);

#define TH_TEST_CASE_END                                                \
    int outstanding = th_test_allocator_outstanding();                  \
    if (outstanding != 0) {                                             \
        printf(" Memory leak detected: %d allocations\n", outstanding); \
        return TH_TEST_FAILURE;                                         \
    }                                                                   \
    printf(" passed\n");                                                \
    }

#endif
