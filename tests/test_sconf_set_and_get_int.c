#include <setjmp.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <cmocka.h>

#include "sconf.h"

static void test_sconf_set_and_get_integer(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_set_int(root, "number", 1, &err);
    assert_int_equal(r, 0);

    const int64_t *integer;
    r = sconf_get_int(root, "number", &integer, &err);
    assert_int_equal(r, 1);
    assert_int_equal(*integer, 1);

    sconf_node_destroy(root);
}

static void test_sconf_get_integer_not_found(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    const int64_t *integer;
    int r = sconf_get_int(root, "d.o.e.s.n.o.t.e.x.i.s.t", &integer, &err);
    assert_int_equal(r, 0);

    sconf_node_destroy(root);
}

static void test_sconf_get_integer_wrong_type(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_set_str(root, "bar", "foo", &err);
    assert_int_equal(r, 0);

    const int64_t *integer;
    r = sconf_get_int(root, "bar", &integer, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_get_integer_without_root(void **unused)
{
    struct SConfErr err = {0};

    const int64_t *integer;
    int r = sconf_get_int(NULL, "b", &integer, &err);
    assert_int_equal(r, -1);
}

static void test_sconf_get_integer_without_path(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    const int64_t *integer;
    int r = sconf_get_int(root, NULL, &integer, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_get_integer_wrong_parent_type(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_set_str(root, "a.b", "lol", &err);
    assert_int_equal(r, 0);

    const int64_t *integer;
    r = sconf_get_int(root, "a.b.c", &integer, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_get_integer_another_wrong_parent_type(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_set_str(root, "a.b", "lol", &err);
    assert_int_equal(r, 0);

    const int64_t *integer;
    r = sconf_get_int(root, "a.b.c.d", &integer, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_set_integer_overwrite_existing(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_set_int(root, "number", 666, &err);
    assert_int_equal(r, 0);

    const int64_t *integer;
    r = sconf_get_int(root, "number", &integer, &err);
    assert_int_equal(r, 1);
    assert_int_equal(*integer, 666);

    r = sconf_set_int(root, "number", 777, &err);
    assert_int_equal(r, 0);

    r = sconf_get_int(root, "number", &integer, &err);
    assert_int_equal(r, 1);
    assert_int_equal(*integer, 777);

    sconf_node_destroy(root);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_sconf_set_and_get_integer),
        cmocka_unit_test(test_sconf_get_integer_not_found),
        cmocka_unit_test(test_sconf_get_integer_wrong_type),
        cmocka_unit_test(test_sconf_get_integer_without_root),
        cmocka_unit_test(test_sconf_get_integer_without_path),
        cmocka_unit_test(test_sconf_get_integer_wrong_parent_type),
        cmocka_unit_test(test_sconf_get_integer_another_wrong_parent_type),
        cmocka_unit_test(test_sconf_set_integer_overwrite_existing),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}

