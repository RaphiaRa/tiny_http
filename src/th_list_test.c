#include "th_list.h"
#include "th_test.h"

#include <stdint.h>

typedef struct th_test_node {
    struct th_test_node* next;
    struct th_test_node* prev;
} th_test_node;

TH_DEFINE_LIST(th_test_list, th_test_node, prev, next)

TH_TEST_BEGIN(list)
{
    TH_TEST_CASE_BEGIN(hashmap_init)
    {
        th_test_list list = {0};
        th_test_node* node = th_test_list_pop_front(&list);
        TH_EXPECT(node == NULL);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(list_push_back)
    {
        th_test_list list = {0};
        th_test_node node1 = {0};
        th_test_list_push_back(&list, &node1);
        th_test_node* node = th_test_list_pop_front(&list);
        TH_EXPECT(node == &node1);
        TH_EXPECT(list.head == NULL);
        TH_EXPECT(list.tail == NULL);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(list_push_back_multiple)
    {
        th_test_list list = {0};
        th_test_node node1 = {0};
        th_test_node node2 = {0};
        th_test_list_push_back(&list, &node1);
        th_test_list_push_back(&list, &node2);
        th_test_node* node = th_test_list_pop_front(&list);
        TH_EXPECT(node == &node1);
        node = th_test_list_pop_front(&list);
        TH_EXPECT(node == &node2);
        TH_EXPECT(list.head == NULL);
        TH_EXPECT(list.tail == NULL);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(list_erase)
    {
        th_test_list list = {0};
        th_test_node node1 = {0};
        th_test_node node2 = {0};
        th_test_list_push_back(&list, &node1);
        th_test_list_push_back(&list, &node2);
        th_test_list_erase(&list, &node1);
        th_test_node* node = th_test_list_pop_front(&list);
        TH_EXPECT(node == &node2);
        TH_EXPECT(list.head == NULL);
        TH_EXPECT(list.tail == NULL);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(list_erase_middle)
    {
        th_test_list list = {0};
        th_test_node node1 = {0};
        th_test_node node2 = {0};
        th_test_node node3 = {0};
        th_test_list_push_back(&list, &node1);
        th_test_list_push_back(&list, &node2);
        th_test_list_push_back(&list, &node3);
        th_test_list_erase(&list, &node2);
        th_test_node* node = th_test_list_pop_front(&list);
        TH_EXPECT(node == &node1);
        node = th_test_list_pop_front(&list);
        TH_EXPECT(node == &node3);
        TH_EXPECT(list.head == NULL);
        TH_EXPECT(list.tail == NULL);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(list_push_erase)
    {
        th_test_list list = {0};
        th_test_node node1 = {0};
        th_test_node node2 = {0};
        th_test_node node3 = {0};
        th_test_list_push_back(&list, &node1);
        th_test_list_push_back(&list, &node2);
        th_test_list_push_back(&list, &node3);
        th_test_list_erase(&list, &node3);
        th_test_list_erase(&list, &node1);
        th_test_list_erase(&list, &node2);
        th_test_node* node = th_test_list_pop_front(&list);
        TH_EXPECT(node == NULL);
        th_test_list_push_back(&list, &node1);
        node = th_test_list_pop_front(&list);
        TH_EXPECT(node == &node1);
        node = th_test_list_pop_front(&list);
        TH_EXPECT(node == NULL);
    }
    TH_TEST_CASE_END
}
TH_TEST_END
