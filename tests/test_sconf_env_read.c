#include <setjmp.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <cmocka.h>

#include "sconf.h"

static void test_sconf_env_read_string(void **unused)
{
    static struct SConfMap map[] = {
        {
            .path = "foo",
            .type = SCONF_TYPE_STR,
            .env = "SCONF_TEST_STR_FOO",
        },
        {0}
    };

    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = putenv("SCONF_TEST_STR_FOO=Hello, world!");
    assert_int_equal(r, 0);

    r = sconf_env_read(root, map, &err);
    assert_int_equal(r, 0);

    const char *string;
    r = sconf_get_str(root, "foo", &string, &err);
    assert_int_equal(r, 1);
    assert_string_equal(string, "Hello, world!");

    sconf_node_destroy(root);
}

static void test_sconf_env_read_string_not_set(void **unused)
{
    static struct SConfMap map[] = {
        {
            .path = "foo",
            .type = SCONF_TYPE_STR,
            .env = "SCONF_TEST_STR_NO_FOO",
        },
        {0}
    };

    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_env_read(root, map, &err);
    assert_int_equal(r, 0);

    const char *string;
    r = sconf_get_str(root, "foo", &string, &err);
    assert_int_equal(r, 0);

    sconf_node_destroy(root);
}

static void test_sconf_env_read_integer(void **unused)
{
    static struct SConfMap map[] = {
        {
            .path = "bar",
            .type = SCONF_TYPE_INT,
            .env = "SCONF_TEST_INT_BAR",
        },
        {0}
    };

    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = putenv("SCONF_TEST_INT_BAR=313");
    assert_int_equal(r, 0);

    r = sconf_env_read(root, map, &err);
    assert_int_equal(r, 0);

    const int64_t *integer;
    r = sconf_get_int(root, "bar", &integer, &err);
    assert_int_equal(r, 1);
    assert_int_equal(*integer, 313);

    sconf_node_destroy(root);
}

static void test_sconf_env_read_integer_not_set(void **unused)
{
    static struct SConfMap map[] = {
        {
            .path = "bar",
            .type = SCONF_TYPE_INT,
            .env = "SCONF_TEST_INT_NO_BAR",
        },
        {0}
    };

    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_env_read(root, map, &err);
    assert_int_equal(r, 0);

    const int64_t *integer;
    r = sconf_get_int(root, "bar", &integer, &err);
    assert_int_equal(r, 0);

    sconf_node_destroy(root);
}

static void test_sconf_env_read_integer_type_mismatch(void **unused)
{
    static struct SConfMap map[] = {
        {
            .path = "bar",
            .type = SCONF_TYPE_INT,
            .env = "SCONF_TEST_INT_WRONG_TYPE_BAR",
        },
        {0}
    };

    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = putenv("SCONF_TEST_INT_WRONG_TYPE_BAR=foobar");
    assert_int_equal(r, 0);

    r = sconf_env_read(root, map, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_env_read_float(void **unused)
{
    static struct SConfMap map[] = {
        {
            .path = "meh",
            .type = SCONF_TYPE_FLOAT,
            .env = "SCONF_TEST_FLOAT_MEH",
        },
        {0}
    };

    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = putenv("SCONF_TEST_FLOAT_MEH=12345.678");
    assert_int_equal(r, 0);

    r = sconf_env_read(root, map, &err);
    assert_int_equal(r, 0);

    const double *fp;
    r = sconf_get_float(root, "meh", &fp, &err);
    assert_int_equal(r, 1);
    assert_float_equal(*fp, 12345.678, 0.0);

    sconf_node_destroy(root);
}

static void test_sconf_env_read_float_not_set(void **unused)
{
    static struct SConfMap map[] = {
        {
            .path = "meh",
            .type = SCONF_TYPE_FLOAT,
            .env = "SCONF_TEST_FLOAT_NO_MEH",
        },
        {0}
    };

    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_env_read(root, map, &err);
    assert_int_equal(r, 0);

    const double *fp;
    r = sconf_get_float(root, "meh", &fp, &err);
    assert_int_equal(r, 0);

    sconf_node_destroy(root);
}

static void test_sconf_env_read_float_type_mismatch(void **unused)
{
    static struct SConfMap map[] = {
        {
            .path = "meh",
            .type = SCONF_TYPE_FLOAT,
            .env = "SCONF_TEST_FLOAT_WRONG_TYPE_MEH",
        },
        {0}
    };

    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = putenv("SCONF_TEST_FLOAT_WRONG_TYPE_MEH=false");
    assert_int_equal(r, 0);

    r = sconf_env_read(root, map, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_env_read_boolean(void **unused)
{
    static struct SConfMap map[] = {
        {
            .path = "b",
            .type = SCONF_TYPE_BOOL,
            .env = "SCONF_TEST_BOOL_B",
        },
        {0}
    };

    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = putenv("SCONF_TEST_BOOL_B=true");
    assert_int_equal(r, 0);

    r = sconf_env_read(root, map, &err);
    assert_int_equal(r, 0);

    const bool *boolean;
    r = sconf_get_bool(root, "b", &boolean, &err);
    assert_int_equal(r, 1);
    assert_true(*boolean);

    sconf_node_destroy(root);
}

static void test_sconf_env_read_boolean_not_set(void **unused)
{
    static struct SConfMap map[] = {
        {
            .path = "b",
            .type = SCONF_TYPE_BOOL,
            .env = "SCONF_TEST_BOOL_NO_B",
        },
        {0}
    };

    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_env_read(root, map, &err);
    assert_int_equal(r, 0);

    const bool *boolean;
    r = sconf_get_bool(root, "b", &boolean, &err);
    assert_int_equal(r, 0);

    sconf_node_destroy(root);
}

static void test_sconf_env_read_boolean_type_mismatch(void **unused)
{
    static struct SConfMap map[] = {
        {
            .path = "b",
            .type = SCONF_TYPE_BOOL,
            .env = "SCONF_TEST_BOOL_WRONG_TYPE_B",
        },
        {0}
    };

    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = putenv("SCONF_TEST_BOOL_WRONG_TYPE_B=505");
    assert_int_equal(r, 0);

    r = sconf_env_read(root, map, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_env_read_yaml_file(void **unused)
{
    static struct SConfMap map[] = {
        {
            .path = "config",
            .type = SCONF_TYPE_YAML_FILE,
            .env = "SCONF_TEST_YAML_FILE",
        },
        {0}
    };

    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = putenv("SCONF_TEST_YAML_FILE=yaml/test_integer.yaml");
    assert_int_equal(r, 0);

    r = sconf_env_read(root, map, &err);
    assert_int_equal(r, 0);

    const int64_t *integer;
    r = sconf_get_int(root, "a", &integer, &err);
    assert_int_equal(r, 1);
    assert_int_equal(*integer, 1234567890);

    sconf_node_destroy(root);
}

static void test_sconf_env_read_non_existing_yaml_file(void **unused)
{
    static struct SConfMap map[] = {
        {
            .path = "config",
            .type = SCONF_TYPE_YAML_FILE,
            .env = "SCONF_TEST_NON_EXISTING_YAML_FILE",
        },
        {0}
    };

    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = putenv("SCONF_TEST_NON_EXISTING_YAML_FILE=does-not-exist");
    assert_int_equal(r, 0);

    r = sconf_env_read(root, map, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_env_read_missing_root(void **unused)
{
    static struct SConfMap map[] = {
        {
            .path = "a",
            .type = SCONF_TYPE_STR,
            .env = "FOO",
        },
        {0}
    };

    struct SConfErr err = {0};

    int r = sconf_env_read(NULL, map, &err);
    assert_int_equal(r, -1);
}

static void test_sconf_env_read_missing_map(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_env_read(root, NULL, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_env_read_missing_path(void **unused)
{
    static struct SConfMap map[] = {
        {
            .type = SCONF_TYPE_STR,
            .env = "FOO",
        },
        {0}
    };

    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_env_read(root, map, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_env_read_no_env_in_map(void **unused)
{
    static struct SConfMap map[] = {
        {
            .path = "foo",
            .type = SCONF_TYPE_STR,
        },
        {0}
    };

    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_env_read(root, map, &err);
    assert_int_equal(r, 0);

    sconf_node_destroy(root);
}

static void test_sconf_env_read_unsupported_type(void **unused)
{
    static struct SConfMap map[] = {
        {
            .path = "foo",
            .type = SCONF_TYPE_DICT,
            .env = "NO_NO_NO",
        },
        {0}
    };

    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = putenv("NO_NO_NO=no");
    assert_int_equal(r, 0);

    r = sconf_env_read(root, map, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_sconf_env_read_string),
        cmocka_unit_test(test_sconf_env_read_string_not_set),
        cmocka_unit_test(test_sconf_env_read_integer),
        cmocka_unit_test(test_sconf_env_read_integer_not_set),
        cmocka_unit_test(test_sconf_env_read_integer_type_mismatch),
        cmocka_unit_test(test_sconf_env_read_float),
        cmocka_unit_test(test_sconf_env_read_float_not_set),
        cmocka_unit_test(test_sconf_env_read_float_type_mismatch),
        cmocka_unit_test(test_sconf_env_read_boolean),
        cmocka_unit_test(test_sconf_env_read_boolean_not_set),
        cmocka_unit_test(test_sconf_env_read_boolean_type_mismatch),
        cmocka_unit_test(test_sconf_env_read_yaml_file),
        cmocka_unit_test(test_sconf_env_read_non_existing_yaml_file),
        cmocka_unit_test(test_sconf_env_read_missing_root),
        cmocka_unit_test(test_sconf_env_read_missing_map),
        cmocka_unit_test(test_sconf_env_read_missing_path),
        cmocka_unit_test(test_sconf_env_read_no_env_in_map),
        cmocka_unit_test(test_sconf_env_read_unsupported_type),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}

