#include <setjmp.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <cmocka.h>

#include "sconf.h"

int validate_func_node_exists(const char *path, const struct SConfNode *node,
                              void *user, struct SConfErr *err)
{
    assert_string_equal(path, "a.b.c");
    assert_non_null(node);

    const char *string = (const char *)user;
    assert_string_equal(string, "testTESTtest");

    return 0;
}

static void test_sconf_validate_func_node_exists(void **unused)
{
    struct SConfMap map[] = {
        {
            .path = "a.b.c",
            .type = SCONF_TYPE_STR,
            .validate_func = &validate_func_node_exists,
        },
        {0}
    };

    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_set_str(root, "a.b.c", "meh", &err);
    assert_int_equal(r, 0);

    char *string = "testTESTtest";
    r = sconf_validate(root, map, string, &err);
    assert_int_equal(r, 0);

    sconf_node_destroy(root);
}

int validate_func_return_error(const char *path, const struct SConfNode *node,
                               void *user, struct SConfErr *err)
{
    return 1;
}

static void test_sconf_validate_func_return_error(void **unused)
{
    struct SConfMap map[] = {
        {
            .path = "bbb",
            .type = SCONF_TYPE_INT,
            .validate_func = &validate_func_return_error,
        },
        {0}
    };

    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_validate(root, map, NULL, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_validate_func_missing_path(void **unused)
{
    struct SConfMap map[] = {
        {
            .type = SCONF_TYPE_INT,
            .validate_func = &validate_func_return_error,
        },
        {0}
    };

    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_validate(root, map, NULL, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_validate_missing_root(void **unused)
{
    struct SConfMap map[] = {
        {
            .path = "ccc",
            .type = SCONF_TYPE_INT,
        },
        {0}
    };

    struct SConfErr err = {0};

    int r = sconf_validate(NULL, map, NULL, &err);
    assert_int_equal(r, -1);
}

static void test_sconf_validate_missing_map(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_validate(root, NULL, NULL, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_validate_required_existing(void **unused)
{
    struct SConfMap map[] = {
        {
            .path = "ccc",
            .type = SCONF_TYPE_INT,
            .required = true,
        },
        {0}
    };

    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_set_int(root, "ccc", 54321, &err);
    assert_int_equal(r, 0);

    r = sconf_validate(root, map, NULL, &err);
    assert_int_equal(r, 0);

    sconf_node_destroy(root);
}

static void test_sconf_validate_required_non_existing(void **unused)
{
    struct SConfMap map[] = {
        {
            .path = "ccc",
            .type = SCONF_TYPE_INT,
            .required = true,
        },
        {0}
    };

    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_validate(root, map, NULL, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_validate_required_missing_path(void **unused)
{
    struct SConfMap map[] = {
        {
            .type = SCONF_TYPE_FLOAT,
            .required = true,
        },
        {0}
    };

    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_validate(root, map, NULL, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_validate_required_type_mismatch(void **unused)
{
    struct SConfMap map[] = {
        {
            .path = "yyyyy",
            .type = SCONF_TYPE_BOOL,
            .required = true,
        },
        {0}
    };

    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_set_int(root, "yyyyy", 0x10, &err);
    assert_int_equal(r, 0);

    r = sconf_validate(root, map, NULL, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_sconf_validate_func_node_exists),
        cmocka_unit_test(test_sconf_validate_func_return_error),
        cmocka_unit_test(test_sconf_validate_func_missing_path),
        cmocka_unit_test(test_sconf_validate_missing_root),
        cmocka_unit_test(test_sconf_validate_missing_map),
        cmocka_unit_test(test_sconf_validate_required_existing),
        cmocka_unit_test(test_sconf_validate_required_non_existing),
        cmocka_unit_test(test_sconf_validate_required_missing_path),
        cmocka_unit_test(test_sconf_validate_required_type_mismatch),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}

