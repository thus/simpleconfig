#include <setjmp.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <cmocka.h>

#include "sconf_tests.h"
#include "sconf.h"

static struct SConfMap default_map[] = {
    {
        .path = "bla.bla",
        .type = SCONF_TYPE_STR,
	.default_value = "lol",
    },
    {
        .path = "foo.bar",
        .type = SCONF_TYPE_INT,
	.default_value = "1337",
    },
    {
        .path = "rofl",
        .type = SCONF_TYPE_FLOAT,
        .default_value = "13.37",
    },
    {
        .path = "is.true",
        .type = SCONF_TYPE_BOOL,
        .default_value = "yes",
    },
    {
        .path = "something",
        .type = SCONF_TYPE_STR,
    },
    {
        .path = "should.not.be.here",
        .type = SCONF_TYPE_ARRAY,
    },
    {
        .path = "config",
        .type = SCONF_TYPE_YAML_FILE,
        .default_value = "yaml/test_integer.yaml",
    },
    {0},
};

static void test_sconf_defaults_non_existing_string(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_defaults(root, default_map, &err);
    assert_int_equal(r, 0);

    const char *string;
    r = sconf_get_str(root, "bla.bla", &string, &err);
    assert_int_equal(r, 1);
    assert_string_equal(string, "lol");

    sconf_node_destroy(root);
}

static void test_sconf_defaults_existing_string(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_set_str(root, "bla.bla", "meh", &err);
    assert_int_equal(r, 0);

    r = sconf_defaults(root, default_map, &err);
    assert_int_equal(r, 0);

    const char *string;
    r = sconf_get_str(root, "bla.bla", &string, &err);
    assert_int_equal(r, 1);
    assert_string_equal(string, "meh");

    sconf_node_destroy(root);
}

static void test_sconf_defaults_non_existing_integer(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_defaults(root, default_map, &err);
    assert_int_equal(r, 0);

    const int64_t *integer;
    r = sconf_get_int(root, "foo.bar", &integer, &err);
    assert_int_equal(r, 1);
    assert_int_equal(*integer, 1337);

    sconf_node_destroy(root);
}

static void test_sconf_defaults_existing_integer(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_set_int(root, "foo.bar", 42, &err);
    assert_int_equal(r, 0);

    r = sconf_defaults(root, default_map, &err);
    assert_int_equal(r, 0);

    const int64_t *integer;
    r = sconf_get_int(root, "foo.bar", &integer, &err);
    assert_int_equal(r, 1);
    assert_int_equal(*integer, 42);

    sconf_node_destroy(root);
}

static void test_sconf_defaults_integer_type_mismatch(void **unused)
{
    static struct SConfMap map[] = {
        {
            .path = "foo.bar",
            .type = SCONF_TYPE_INT,
            .default_value = "no",
        },
        {0}
    };

    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_defaults(root, map, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_defaults_non_existing_float(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_defaults(root, default_map, &err);
    assert_int_equal(r, 0);

    const double *fp;
    r = sconf_get_float(root, "rofl", &fp, &err);
    assert_int_equal(r, 1);
    assert_float_equal(*fp, 13.37, 0.0);

    sconf_node_destroy(root);
}

static void test_sconf_defaults_existing_float(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_set_float(root, "rofl", 400.3, &err);
    assert_int_equal(r, 0);

    r = sconf_defaults(root, default_map, &err);
    assert_int_equal(r, 0);

    const double *fp;
    r = sconf_get_float(root, "rofl", &fp, &err);
    assert_int_equal(r, 1);
    assert_float_equal(*fp, 400.3, 0.0);

    sconf_node_destroy(root);
}

static void test_sconf_defaults_float_type_mismatch(void **unused)
{
    static struct SConfMap map[] = {
        {
            .path = "rofl",
            .type = SCONF_TYPE_FLOAT,
            .default_value = "hello",
        },
        {0}
    };

    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_defaults(root, map, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_defaults_non_existing_boolean(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_defaults(root, default_map, &err);
    assert_int_equal(r, 0);

    const bool *boolean;
    r = sconf_get_bool(root, "is.true", &boolean, &err);
    assert_int_equal(r, 1);
    assert_true(*boolean);

    sconf_node_destroy(root);
}

static void test_sconf_defaults_existing_boolean(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_set_bool(root, "is.true", false, &err);
    assert_int_equal(r, 0);

    r = sconf_defaults(root, default_map, &err);
    assert_int_equal(r, 0);

    const bool *boolean;
    r = sconf_get_bool(root, "is.true", &boolean, &err);
    assert_int_equal(r, 1);
    assert_false(*boolean);

    sconf_node_destroy(root);
}

static void test_sconf_defaults_boolean_type_mismatch(void **unused)
{
    static struct SConfMap map[] = {
        {
            .path = "is.true",
            .type = SCONF_TYPE_BOOL,
            .default_value = "0123456789",
        },
        {0}
    };

    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_defaults(root, map, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_defaults_yaml_file(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_defaults(root, default_map, &err);
    assert_int_equal(r, 0);

    const int64_t *integer;
    r = sconf_get_int(root, "a", &integer, &err);
    assert_int_equal(r, 1);
    assert_int_equal(*integer, 1234567890);

    sconf_node_destroy(root);
}

static void test_sconf_defaults_yaml_file_already_read(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_set_str(root, "config", "/path/to/some/file.yaml", &err);
    assert_int_equal(r, 0);

    r = sconf_defaults(root, default_map, &err);
    assert_int_equal(r, 0);

    const int64_t *integer;
    r = sconf_get_int(root, "a", &integer, &err);
    assert_int_equal(r, 0);

    sconf_node_destroy(root);
}

static void test_sconf_defaults_yaml_file_wrong_path(void **unused)
{
    static struct SConfMap map[] = {
        {
            .path = "config",
            .type = SCONF_TYPE_YAML_FILE,
            .default_value = "does-not-exist.yaml",
        },
        {0}
    };

    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_defaults(root, map, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_defaults_missing_root(void **unused)
{
    struct SConfErr err = {0};

    int r = sconf_defaults(NULL, default_map, &err);
    assert_int_equal(r, -1);
}

static void test_sconf_defaults_missing_map(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_defaults(root, NULL, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_defaults_missing_path(void **unused)
{
    static struct SConfMap map[] = {
        {
            .type = SCONF_TYPE_STR,
            .default_value = "roflcopter",
        },
        {0}
    };

    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_defaults(root, map, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_defaults_unsupported_type(void **unused)
{
    static struct SConfMap map[] = {
        {
            .path = "a.b.c",
            .type = SCONF_TYPE_DICT,
            .default_value = "lorem ipsum",
        },
        {0}
    };

    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_defaults(root, map, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_sconf_defaults_non_existing_string),
        cmocka_unit_test(test_sconf_defaults_existing_string),
        cmocka_unit_test(test_sconf_defaults_non_existing_integer),
        cmocka_unit_test(test_sconf_defaults_existing_integer),
        cmocka_unit_test(test_sconf_defaults_integer_type_mismatch),
        cmocka_unit_test(test_sconf_defaults_non_existing_float),
        cmocka_unit_test(test_sconf_defaults_existing_float),
        cmocka_unit_test(test_sconf_defaults_float_type_mismatch),
        cmocka_unit_test(test_sconf_defaults_non_existing_boolean),
        cmocka_unit_test(test_sconf_defaults_existing_boolean),
        cmocka_unit_test(test_sconf_defaults_boolean_type_mismatch),
        cmocka_unit_test(test_sconf_defaults_yaml_file),
        cmocka_unit_test(test_sconf_defaults_yaml_file_already_read),
        cmocka_unit_test(test_sconf_defaults_yaml_file_wrong_path),
        cmocka_unit_test(test_sconf_defaults_missing_root),
        cmocka_unit_test(test_sconf_defaults_missing_map),
        cmocka_unit_test(test_sconf_defaults_missing_path),
        cmocka_unit_test(test_sconf_defaults_unsupported_type),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}

