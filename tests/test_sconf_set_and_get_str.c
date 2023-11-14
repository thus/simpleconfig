#include <setjmp.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <cmocka.h>

#include "sconf.h"

static void test_sconf_set_and_get_string(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_set_str(root, "woop", "foop", &err);
    assert_int_equal(r, 0);

    const char *string = NULL;
    r = sconf_get_str(root, "woop", &string, &err);
    assert_int_equal(r, 1);
    assert_string_equal(string, "foop");

    sconf_node_destroy(root);
}

static void test_sconf_set_and_get_nested_dict_string(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_set_str(root, "w.o.o.p", "foop", &err);
    assert_int_equal(r, 0);

    const char *string = NULL;
    r = sconf_get_str(root, "w.o.o.p", &string, &err);
    assert_int_equal(r, 1);
    assert_string_equal(string, "foop");

    sconf_node_destroy(root);
}

static void test_sconf_set_and_get_nested_array_string(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_set_str(root, "w.[0].[0].p.[50]", "foop", &err);
    assert_int_equal(r, 0);

    const char *string = NULL;
    r = sconf_get_str(root, "w.[0].[0].p.[50]", &string, &err);
    assert_int_equal(r, 1);
    assert_string_equal(string, "foop");

    sconf_node_destroy(root);
}

static void test_sconf_get_string_not_found(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    const char *string = NULL;
    int r = sconf_get_str(root, "d.o.e.s.n.o.t.e.x.i.s.t", &string, &err);
    assert_int_equal(r, 0);

    sconf_node_destroy(root);
}

static void test_sconf_get_string_wrong_type(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_set_int(root, "a", 1814, &err);
    assert_int_equal(r, 0);

    const char *string = NULL;
    r = sconf_get_str(root, "a", &string, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_set_string_without_root(void **unused)
{
    struct SConfErr err = {0};

    int r = sconf_set_str(NULL, "b", "c", &err);
    assert_int_equal(r, -1);
}

static void test_sconf_set_string_without_path(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_set_str(root, NULL, "c", &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_set_string_without_string(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_set_str(root, "d", NULL, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_set_string_overwrite_existing(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_set_str(root, "pw", "ushaen5ahdoushao4Que", &err);
    assert_int_equal(r, 0);

    const char *string = NULL;
    r = sconf_get_str(root, "pw", &string, &err);
    assert_int_equal(r, 1);
    assert_string_equal(string, "ushaen5ahdoushao4Que");

    r = sconf_set_str(root, "pw", "la5Ool3ia2Choojaichi", &err);
    assert_int_equal(r, 0);

    r = sconf_get_str(root, "pw", &string, &err);
    assert_int_equal(r, 1);
    assert_string_equal(string, "la5Ool3ia2Choojaichi");

    sconf_node_destroy(root);
}

static void test_sconf_set_string_reach_max_depth(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_set_str(root, "a.b.c.d.e.f.g.h.i.j.k.l.m.n.o.p.q.r.s", "meh", &err);
    assert_int_equal(r, 0);

    r = sconf_set_str(root, "b.b.c.d.e.f.g.h.i.j.k.l.m.n.o.p.q.r.s.t", "lol", &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_sconf_set_and_get_string),
        cmocka_unit_test(test_sconf_set_and_get_nested_dict_string),
        cmocka_unit_test(test_sconf_set_and_get_nested_array_string),
        cmocka_unit_test(test_sconf_get_string_not_found),
        cmocka_unit_test(test_sconf_get_string_wrong_type),
        cmocka_unit_test(test_sconf_set_string_without_root),
        cmocka_unit_test(test_sconf_set_string_without_path),
        cmocka_unit_test(test_sconf_set_string_without_string),
        cmocka_unit_test(test_sconf_set_string_overwrite_existing),
        cmocka_unit_test(test_sconf_set_string_reach_max_depth),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}

