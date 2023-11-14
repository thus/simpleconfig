#include <errno.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <cmocka.h>

#include "sconf.h"

static void test_sconf_set_and_get_bool(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_set_bool(root, "enabled", true, &err);
    assert_int_equal(r, 0);

    const bool *boolean;
    r = sconf_get_bool(root, "enabled", &boolean, &err);
    assert_int_equal(r, 1);
    assert_true(*boolean);

    sconf_node_destroy(root);
}

static void test_sconf_get_bool_not_found(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    const bool *boolean;
    int r = sconf_get_bool(root, "d.o.e.s.n.o.t.e.x.i.s.t", &boolean, &err);
    assert_int_equal(r, 0);

    sconf_node_destroy(root);
}

static void test_sconf_get_bool_wrong_type(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_set_int(root, "foo", 1881, &err);
    assert_int_equal(r, 0);

    const bool *boolean;
    r = sconf_get_bool(root, "foo", &boolean, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_get_bool_array_index_missing_digits(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    bool boolean = true;
    int r = sconf_set_bool(root, "arr.[]", boolean, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_get_bool_array_index_missing_end_bracket(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    bool boolean = true;
    int r = sconf_set_bool(root, "arr.[14", boolean, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_get_bool_array_invalid_index(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    bool boolean = true;
    int r = sconf_set_bool(root, "arr.[a]", boolean, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_get_bool_array_index_out_of_bounds(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    bool boolean = true;
    int r = sconf_set_bool(root, "arr.[99999999999999999999999999999]", boolean, &err);
    assert_int_equal(errno, ERANGE);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_set_bool_overwrite_existing(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_set_bool(root, "enabled", true, &err);
    assert_int_equal(r, 0);

    const bool *boolean;
    r = sconf_get_bool(root, "enabled", &boolean, &err);
    assert_int_equal(r, 1);
    assert_true(*boolean);

    r = sconf_set_bool(root, "enabled", false, &err);
    assert_int_equal(r, 0);

    r = sconf_get_bool(root, "enabled", &boolean, &err);
    assert_int_equal(r, 1);
    assert_false(*boolean);

    sconf_node_destroy(root);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_sconf_set_and_get_bool),
        cmocka_unit_test(test_sconf_get_bool_not_found),
        cmocka_unit_test(test_sconf_get_bool_wrong_type),
        cmocka_unit_test(test_sconf_get_bool_array_index_missing_digits),
        cmocka_unit_test(test_sconf_get_bool_array_index_missing_end_bracket),
        cmocka_unit_test(test_sconf_get_bool_array_invalid_index),
        cmocka_unit_test(test_sconf_get_bool_array_index_out_of_bounds),
        cmocka_unit_test(test_sconf_set_bool_overwrite_existing),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}

