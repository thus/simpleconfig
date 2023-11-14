#include <setjmp.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <cmocka.h>

#include "sconf.h"

int array_iterator_cb(uint32_t index, struct SConfNode *node, void *user,
                      struct SConfErr *err)
{
    assert_non_null(node);
    assert_int_not_equal(index, 2);

    int *count = (int *)user;
    *count += 1;

    return 0;
}

static void test_sconf_node_array_foreach_successful(void **unused)
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

    r = sconf_node_array_foreach(array, &array_iterator_cb, &count, &err);
    assert_int_equal(r, 0);
    assert_int_equal(count, 3);

    sconf_node_destroy(root);
}

int array_iterator_error_cb(uint32_t index, struct SConfNode *node, void *user,
                            struct SConfErr *err)
{
    sconf_err_set(err, "All your base!");
    return -2;
}

static void test_sconf_node_array_foreach_return_error(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_set_str(root, "a.[0]", "abcde", &err);
    assert_int_equal(r, 0);

    struct SConfNode *array = NULL;
    r = sconf_get(root, "a", &array, &err);
    assert_int_equal(r, 1);
    assert_non_null(array);

    r = sconf_node_array_foreach(array, &array_iterator_error_cb, NULL, &err);
    assert_int_equal(r, -1);
    assert_string_equal(sconf_strerror(&err), "All your base!");

    sconf_node_destroy(root);
}

static void test_sconf_node_array_foreach_array_missing(void **unused)
{
    struct SConfErr err = {0};

    int r = sconf_node_array_foreach(NULL, &array_iterator_error_cb, NULL, &err);
    assert_int_equal(r, -1);
}

static void test_sconf_node_array_foreach_callback_missing(void **unused)
{
    struct SConfNode *array = sconf_node_create(SCONF_TYPE_ARRAY, NULL, NULL);
    assert_non_null(array);

    struct SConfErr err = {0};

    int r = sconf_node_array_foreach(array, NULL, NULL, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(array);
}

static void test_sconf_node_array_foreach_wrong_type(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_node_array_foreach(root, &array_iterator_error_cb, NULL,
                                     &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_sconf_node_array_foreach_successful),
        cmocka_unit_test(test_sconf_node_array_foreach_return_error),
        cmocka_unit_test(test_sconf_node_array_foreach_array_missing),
        cmocka_unit_test(test_sconf_node_array_foreach_callback_missing),
        cmocka_unit_test(test_sconf_node_array_foreach_wrong_type),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}

