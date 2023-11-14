#include <setjmp.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <cmocka.h>

#include "sconf.h"

static void test_sconf_array_insert_and_search_successful(void **unused)
{
    struct SConfNode *array = sconf_node_create(SCONF_TYPE_ARRAY, NULL, NULL);
    assert_non_null(array);

    int64_t integer = 10101010;
    struct SConfNode *n1 = sconf_node_create(SCONF_TYPE_INT, &integer, NULL);
    assert_non_null(n1);

    int r = sconf_node_array_insert(2, array, n1, NULL);
    assert_int_equal(r, 0);

    struct SConfNode *n2 = NULL;
    struct SConfNode *n3 = NULL;
    struct SConfNode *n4 = NULL;

    /* Should not exist */
    r = sconf_node_array_search(0, array, &n2, NULL);
    assert_int_equal(r, 0);
    assert_null(n2);

    /* Should not exist */
    r = sconf_node_array_search(1, array, &n3, NULL);
    assert_int_equal(r, 0);
    assert_null(n3);

    /* Should return the node we inserted earlier */
    r = sconf_node_array_search(2, array, &n4, NULL);
    assert_int_equal(r, 0);
    assert_non_null(n4);
    assert_int_equal(sconf_int(n4), 10101010);

    sconf_node_destroy(array);
}

static void test_sconf_array_search_not_found(void **unused)
{
    struct SConfNode *array = sconf_node_create(SCONF_TYPE_ARRAY, NULL, NULL);
    assert_non_null(array);

    struct SConfNode *node = NULL;
    int r = sconf_node_array_search(0, array, &node, NULL);
    assert_int_equal(r, 0);
    assert_null(node);

    sconf_node_destroy(array);
}

static void test_sconf_array_insert_parent_missing(void **unused)
{
    char *string = "foo";
    struct SConfNode *node = sconf_node_create(SCONF_TYPE_STR, string, NULL);
    assert_non_null(node);

    int r = sconf_node_array_insert(0, NULL, node, NULL);
    assert_int_equal(r, -1);

    sconf_node_destroy(node);
}

static void test_sconf_array_insert_node_missing(void **unused)
{
    struct SConfNode *array = sconf_node_create(SCONF_TYPE_ARRAY, NULL, NULL);
    assert_non_null(array);

    int r = sconf_node_array_insert(0, array, NULL, NULL);
    assert_int_equal(r, -1);

    sconf_node_destroy(array);
}

static void test_sconf_array_insert_parent_type_not_array(void **unused)
{
    char *string = "foo";
    struct SConfNode *parent = sconf_node_create(SCONF_TYPE_STR, string, NULL);
    assert_non_null(parent);

    int64_t integer = 123;
    struct SConfNode *node = sconf_node_create(SCONF_TYPE_INT, &integer, NULL);
    assert_non_null(node);

    int r = sconf_node_array_insert(0, parent, node, NULL);
    assert_int_equal(r, -1);

    sconf_node_destroy(parent);
    sconf_node_destroy(node);
}

static void test_sconf_array_search_parent_missing(void **unused)
{
    struct SConfNode *node = NULL;
    int r = sconf_node_array_search(0, NULL, &node, NULL);
    assert_int_equal(r, -1);
    assert_null(node);
}

static void test_sconf_array_search_parent_type_not_array(void **unused)
{
    char *string = "foo";
    struct SConfNode *parent = sconf_node_create(SCONF_TYPE_STR, string, NULL);
    assert_non_null(parent);

    struct SConfNode *node = NULL;
    int r = sconf_node_array_search(0, parent, &node, NULL);
    assert_int_equal(r, -1);

    sconf_node_destroy(parent);
}

static void test_sconf_set_with_array_as_root(void **unused)
{
    /* Used to fix a bug in `sconf_set` where the parent index is
       calculated to late when the first element is an array index. */ 
    struct SConfNode *array = sconf_node_create(SCONF_TYPE_ARRAY, NULL, NULL);
    assert_non_null(array);

    char *string = "meh";
    int r = sconf_set_str(array, "[1]", string, NULL);
    assert_int_equal(r, 0);

    struct SConfNode *node = NULL;
    r = sconf_node_array_search(1, array, &node, NULL);
    assert_int_equal(r, 0);
    assert_non_null(node);

    sconf_node_destroy(array);
}

static void test_sconf_get_with_array_as_root(void **unused)
{
    /* Used to fix a bug in `sconf_get` where the parent index is
       calculated to late when the first element is an array index. */
    struct SConfNode *array = sconf_node_create(SCONF_TYPE_ARRAY, NULL, NULL);
    assert_non_null(array);

    char *string = "meh";
    int r = sconf_set_str(array, "[10]", string, NULL);
    assert_int_equal(r, 0);

    const char *tmp = NULL;
    r = sconf_get_str(array, "[10]", &tmp, NULL);
    assert_int_equal(r, 1);
    assert_string_equal(tmp, "meh");

    sconf_node_destroy(array);
}

static void test_sconf_set_array_index_out_of_bounds(void **unused)
{
    struct SConfErr err = {0};

    struct SConfNode *array = sconf_node_create(SCONF_TYPE_ARRAY, NULL, &err);
    assert_non_null(array);

    char *string = "meh";
    int r = sconf_set_str(array, "[65536]", string, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(array);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_sconf_array_insert_and_search_successful),
        cmocka_unit_test(test_sconf_array_search_not_found),
        cmocka_unit_test(test_sconf_array_insert_parent_missing),
        cmocka_unit_test(test_sconf_array_insert_node_missing),
        cmocka_unit_test(test_sconf_array_insert_parent_type_not_array),
        cmocka_unit_test(test_sconf_array_search_parent_missing),
        cmocka_unit_test(test_sconf_array_search_parent_type_not_array),
        cmocka_unit_test(test_sconf_set_with_array_as_root),
        cmocka_unit_test(test_sconf_get_with_array_as_root),
        cmocka_unit_test(test_sconf_set_array_index_out_of_bounds),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}

