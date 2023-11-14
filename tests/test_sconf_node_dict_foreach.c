#include <setjmp.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <cmocka.h>

#include "sconf.h"

int dict_iterator_cb(const unsigned char *name, struct SConfNode *node,
                     void *user, struct SConfErr *err)
{
    assert_non_null(node);

    int *count = (int *)user;
    *count += 1;

    return 0;
}

static void test_sconf_node_dict_foreach_successful(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_set_str(root, "a.b.foo", "abcde", &err);
    assert_int_equal(r, 0);
    r = sconf_set_str(root, "a.b.bar", "fghij", &err);
    assert_int_equal(r, 0);
    r = sconf_set_str(root, "a.b.meh", "klmno", &err);
    assert_int_equal(r, 0);

    struct SConfNode *dict = NULL;
    r = sconf_get(root, "a.b", &dict, &err);
    assert_int_equal(r, 1);
    assert_non_null(dict);

    int count = 0;

    r = sconf_node_dict_foreach(dict, &dict_iterator_cb, &count, &err);
    assert_int_equal(r, 0);
    assert_int_equal(count, 3);

    sconf_node_destroy(root);
}

int dict_iterator_error_cb(const unsigned char *name, struct SConfNode *node,
                           void *user, struct SConfErr *err)
{
    sconf_err_set(err, "oh, noes!");
    return 1;
}

static void test_sconf_node_dict_foreach_return_error(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_set_str(root, "a.b.foo", "abcde", &err);
    assert_int_equal(r, 0);

    struct SConfNode *dict = NULL;
    r = sconf_get(root, "a.b", &dict, &err);
    assert_int_equal(r, 1);
    assert_non_null(dict);

    r = sconf_node_dict_foreach(dict, &dict_iterator_error_cb, NULL, &err);
    assert_int_equal(r, -1);
    assert_string_equal(sconf_strerror(&err), "oh, noes!");

    sconf_node_destroy(root);
}

static void test_sconf_node_dict_foreach_missing_dict(void **unused)
{
    struct SConfErr err = {0};

    int r = sconf_node_dict_foreach(NULL, &dict_iterator_error_cb, NULL, &err);
    assert_int_equal(r, -1);
}

static void test_sconf_node_dict_foreach_missing_callback(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_node_dict_foreach(root, NULL, NULL, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_node_dict_foreach_wrong_node_type(void **unused)
{
    struct SConfNode *node = sconf_node_create(SCONF_TYPE_ARRAY, NULL, NULL);
    assert_non_null(node);

    struct SConfErr err = {0};

    int r = sconf_node_dict_foreach(node, &dict_iterator_error_cb, NULL, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(node);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_sconf_node_dict_foreach_successful),
        cmocka_unit_test(test_sconf_node_dict_foreach_return_error),
        cmocka_unit_test(test_sconf_node_dict_foreach_missing_dict),
        cmocka_unit_test(test_sconf_node_dict_foreach_missing_callback),
        cmocka_unit_test(test_sconf_node_dict_foreach_wrong_node_type),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}

