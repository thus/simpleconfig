#include <setjmp.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <cmocka.h>

#include "sconf.h"

static void test_sconf_set_and_get_float(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_set_float(root, "pi", 3.14, &err);
    assert_int_equal(r, 0);

    const double *fp;
    r = sconf_get_float(root, "pi", &fp, &err);
    assert_int_equal(r, 1);
    assert_float_equal(*fp, 3.14, 0.0);

    sconf_node_destroy(root);
}

static void test_sconf_get_float_not_found(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    const double *fp;
    int r = sconf_get_float(root, "d.o.e.s.n.o.t.e.x.i.s.t", &fp, &err);
    assert_int_equal(r, 0);

    sconf_node_destroy(root);
}

static void test_sconf_get_float_another_not_found(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    /* Let's make sure that all the parents exist this time */
    int r = sconf_set_int(root, "d.o.e.s.n.o.t.e.x.i.s.x", 1881, &err);
    assert_int_equal(r, 0);

    const double *fp;
    r = sconf_get_float(root, "d.o.e.s.n.o.t.e.x.i.s.t", &fp, &err);
    assert_int_equal(r, 0);

    sconf_node_destroy(root);
}

static void test_sconf_get_float_wrong_type(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_set_int(root, "foo", 1881, &err);
    assert_int_equal(r, 0);

    const double *fp;
    r = sconf_get_float(root, "foo", &fp, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_float_overwrite_existing(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_set_float(root, "blabla", 1.2345, &err);
    assert_int_equal(r, 0);

    const double *fp;
    r = sconf_get_float(root, "blabla", &fp, &err);
    assert_int_equal(r, 1);
    assert_float_equal(*fp, 1.2345, 0.0);

    r = sconf_set_float(root, "blabla", 5.4321, &err);
    assert_int_equal(r, 0);

    r = sconf_get_float(root, "blabla", &fp, &err);
    assert_int_equal(r, 1);
    assert_float_equal(*fp, 5.4321, 0.0);

    sconf_node_destroy(root);
}

static void test_sconf_float_overwrite_existing_wrong_type(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_set_int(root, "blabla", 12345, &err);
    assert_int_equal(r, 0);

    r = sconf_set_float(root, "blabla", 5.4321, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_sconf_set_and_get_float),
        cmocka_unit_test(test_sconf_get_float_not_found),
        cmocka_unit_test(test_sconf_get_float_wrong_type),
        cmocka_unit_test(test_sconf_get_float_another_not_found),
        cmocka_unit_test(test_sconf_float_overwrite_existing),
        cmocka_unit_test(test_sconf_float_overwrite_existing_wrong_type),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}

