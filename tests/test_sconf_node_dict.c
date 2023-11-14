#include <setjmp.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <cmocka.h>

#include "sconf.h"

static void test_sconf_dict_insert_and_search_successful(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    char *string = "foobar";
    struct SConfNode *n1 = sconf_node_create(SCONF_TYPE_STR, string, NULL);
    assert_non_null(n1);

    int r = sconf_node_dict_insert("meh", root, n1, NULL);
    assert_int_equal(r, 0);

    struct SConfNode *n2 = NULL;
    r = sconf_node_dict_search("meh", root, &n2, NULL);
    assert_int_equal(r, 0);
    assert_non_null(n2);
    assert_string_equal(sconf_str(n2), "foobar");

    sconf_node_destroy(root);
}

static void test_sconf_dict_search_not_found(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfNode *node = NULL;
    int r = sconf_node_dict_search("oh-noes", root, &node, NULL);
    assert_int_equal(r, 0);
    assert_null(node);

    sconf_node_destroy(root);
}

static void test_sconf_dict_insert_name_missing(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    char *string = "foobar";
    struct SConfNode *node = sconf_node_create(SCONF_TYPE_STR, string, NULL);
    assert_non_null(node);

    int r = sconf_node_dict_insert(NULL, root, node, NULL);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
    sconf_node_destroy(node);
}

static void test_sconf_dict_insert_parent_missing(void **unused)
{
    char *string = "foobar";
    struct SConfNode *node = sconf_node_create(SCONF_TYPE_STR, string, NULL);
    assert_non_null(node);

    int r = sconf_node_dict_insert("meh", NULL, node, NULL);
    assert_int_equal(r, -1);

    sconf_node_destroy(node);
}

static void test_sconf_dict_insert_node_missing(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    int r = sconf_node_dict_insert("meh", root, NULL, NULL);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_dict_insert_parent_type_not_dict(void **unused)
{
    struct SConfNode *parent = sconf_node_create(SCONF_TYPE_STR, "foo", NULL);
    assert_non_null(parent);

    char *string = "foobar";
    struct SConfNode *node = sconf_node_create(SCONF_TYPE_STR, string, NULL);
    assert_non_null(node);

    int r = sconf_node_dict_insert("meh", parent, node, NULL);
    assert_int_equal(r, -1);

    sconf_node_destroy(parent);
    sconf_node_destroy(node);
}

static void test_sconf_dict_search_name_missing(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfNode *node = NULL;
    int r = sconf_node_dict_search(NULL, root, &node, NULL);
    assert_int_equal(r, -1);
    assert_null(node);

    sconf_node_destroy(root);

    /* Just to test destroying nodes that are NULL! */
    sconf_node_destroy(node);
}

static void test_sconf_dict_search_parent_missing(void **unused)
{
    struct SConfNode *node = NULL;
    int r = sconf_node_dict_search("meh", NULL, &node, NULL);
    assert_int_equal(r, -1);
    assert_null(node);
}

static void test_sconf_dict_search_parent_type_not_dict(void **unused)
{
    struct SConfNode *parent = sconf_node_create(SCONF_TYPE_STR, "foo", NULL);
    assert_non_null(parent);

    struct SConfNode *node = NULL;
    int r = sconf_node_dict_search("meh", parent, &node, NULL);
    assert_int_equal(r, -1);
    assert_null(node);

    sconf_node_destroy(parent);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_sconf_dict_insert_and_search_successful),
        cmocka_unit_test(test_sconf_dict_search_not_found),
        cmocka_unit_test(test_sconf_dict_insert_name_missing),
        cmocka_unit_test(test_sconf_dict_insert_parent_missing),
        cmocka_unit_test(test_sconf_dict_insert_node_missing),
        cmocka_unit_test(test_sconf_dict_insert_parent_type_not_dict),
        cmocka_unit_test(test_sconf_dict_search_name_missing),
        cmocka_unit_test(test_sconf_dict_search_parent_missing),
        cmocka_unit_test(test_sconf_dict_search_parent_type_not_dict),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}

