#include <setjmp.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <cmocka.h>

#include "sconf.h"

static void test_sconf_node_array_next_successful(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_set_str(root, "a.[0]", "abcde", &err);
    assert_int_equal(r, 0);
    r = sconf_set_str(root, "a.[1]", "fghij", &err);
    assert_int_equal(r, 0);
    r = sconf_set_str(root, "a.[3]", "klmno", &err);
    assert_int_equal(r, 0);

    struct SConfNode *array = NULL;
    r = sconf_get(root, "a", &array, &err);
    assert_int_equal(r, 1);
    assert_non_null(array);

    int count = 0;

    struct SConfNode *node = NULL;
    uint32_t next = 0;
    while ((r = sconf_node_array_next(array, &node, &next, &err)) == 1)
    {
        count++;
        assert_int_not_equal(next, 3); /* [2] should be missing */
        assert_non_null(node);
    }

    assert_int_equal(r, 0);
    assert_int_equal(count, 3);

    sconf_node_destroy(root);
}

static void test_sconf_node_array_next_array_missing(void **unused)
{
    struct SConfNode *node = NULL;
    uint32_t next = 0;
    int r = sconf_node_array_next(NULL, &node, &next, NULL);
    assert_int_equal(r, -1);
}

static void test_sconf_node_array_next_wrong_type(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    struct SConfNode *node = NULL;
    uint32_t next = 0;
    int r = sconf_node_array_next(root, &node, &next, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_sconf_node_array_next_successful),
        cmocka_unit_test(test_sconf_node_array_next_array_missing),
        cmocka_unit_test(test_sconf_node_array_next_wrong_type),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}

