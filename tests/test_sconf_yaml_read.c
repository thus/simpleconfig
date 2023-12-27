#include <setjmp.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <cmocka.h>

#include "sconf_tests.h"
#include "sconf.h"

static void test_valid_yaml_string(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_yaml_read(root, "yaml/test_string.yaml", &err);
    printf("err: %s\n", sconf_strerror(&err));
    assert_int_equal(r, 0);

    const char *string = NULL;

    /* Test normal string */
    r = sconf_get_str(root, "lol", &string, NULL);
    assert_int_equal(r, 1);
    assert_string_equal(string, "rofl");

    /* Test double-quoted string */
    r = sconf_get_str(root, "foo", &string, NULL);
    assert_int_equal(r, 1);
    assert_string_equal(string, "123");

    /* Test single-quoted string */
    r = sconf_get_str(root, "bar", &string, NULL);
    assert_int_equal(r, 1);
    assert_string_equal(string, "000000000");

    /* Test multi-line string with folded spaces */
    r = sconf_get_str(root, "b1", &string, NULL);
    assert_int_equal(r, 1);
    assert_string_equal(string, "testA testB testC\n");

    /* Test multi-line string with newlines */
    r = sconf_get_str(root, "b2", &string, NULL);
    assert_int_equal(r, 1);
    assert_string_equal(string, "testD\ntestE\ntestF\n");

    /* Test empty string */
    r = sconf_get_str(root, "empty", &string, NULL);
    assert_int_equal(r, 1);
    assert_string_equal(string, "");

    sconf_node_destroy(root);
}

static void test_valid_yaml_integer(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_yaml_read(root, "yaml/test_integer.yaml", &err);
    printf("err: %s\n", sconf_strerror(&err));
    assert_int_equal(r, 0);

    const int64_t *integer;

    /* Test decimal number */
    r = sconf_get_int(root, "a", &integer, NULL);
    assert_int_equal(r, 1);
    assert_int_equal(*integer, 1234567890);

    /* Test octal number */
    r = sconf_get_int(root, "b", &integer, NULL);
    assert_int_equal(r, 1);
    assert_int_equal(*integer, 18);

    /* Test hexadecimal number */
    r = sconf_get_int(root, "c", &integer, NULL);
    assert_int_equal(r, 1);
    assert_int_equal(*integer, 255);

    /* Test zero decimal number */
    r = sconf_get_int(root, "d", &integer, NULL);
    assert_int_equal(r, 1);
    assert_int_equal(*integer, 0);

    /* Test negative decimal number */
    r = sconf_get_int(root, "e", &integer, NULL);
    assert_int_equal(r, 1);
    assert_int_equal(*integer, -1234567890);

    sconf_node_destroy(root);
}

static void test_valid_yaml_boolean(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_yaml_read(root, "yaml/test_boolean.yaml", &err);
    printf("err: %s\n", sconf_strerror(&err));
    assert_int_equal(r, 0);

    const bool *boolean;

    /* Test 'true' */
    r = sconf_get_bool(root, "a", &boolean, NULL);
    assert_int_equal(r, 1);
    assert_true(*boolean);

    /* Test 'false' */
    r = sconf_get_bool(root, "b", &boolean, NULL);
    assert_int_equal(r, 1);
    assert_false(*boolean);

    /* Test 'on' */
    r = sconf_get_bool(root, "c", &boolean, NULL);
    assert_int_equal(r, 1);
    assert_true(*boolean);

    /* Test 'off' */
    r = sconf_get_bool(root, "d", &boolean, NULL);
    assert_int_equal(r, 1);
    assert_false(*boolean);

    /* Test 'yes' */
    r = sconf_get_bool(root, "e", &boolean, NULL);
    assert_int_equal(r, 1);
    assert_true(*boolean);

    /* Test 'no' */
    r = sconf_get_bool(root, "f", &boolean, NULL);
    assert_int_equal(r, 1);
    assert_false(*boolean);

    sconf_node_destroy(root);
}

static void test_valid_yaml_float(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_yaml_read(root, "yaml/test_float.yaml", &err);
    printf("err: %s\n", sconf_strerror(&err));
    assert_int_equal(r, 0);

    const double *fp;

    /* Test low floating-point number */
    r = sconf_get_float(root, "a", &fp, NULL);
    assert_int_equal(r, 1);
    assert_float_equal(*fp, 0.00014, 0.0);

    /* Test high floating-point number */
    r = sconf_get_float(root, "b", &fp, NULL);
    assert_int_equal(r, 1);
    assert_float_equal(*fp, 12000003456789.12345, 0.0);

    /* Test zero floating-point number */
    r = sconf_get_float(root, "c", &fp, NULL);
    assert_int_equal(r, 1);
    assert_float_equal(*fp, 0.0, 0.0);

    /* Test negative floating-point number */
    r = sconf_get_float(root, "d", &fp, NULL);
    assert_int_equal(r, 1);
    assert_float_equal(*fp, -14.3, 0.0);

    sconf_node_destroy(root);
}

static void test_valid_yaml_nested_dicts(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_yaml_read(root, "yaml/test_nested_dicts.yaml", &err);
    printf("err: %s\n", sconf_strerror(&err));
    assert_int_equal(r, 0);

    const char *string = NULL;

    r = sconf_get_str(root, "a.b.c.d.e", &string, NULL);
    assert_int_equal(r, 1);
    assert_string_equal(string, "foo");

    sconf_node_destroy(root);
}

static void test_valid_yaml_array(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_yaml_read(root, "yaml/test_array.yaml", &err);
    printf("err: %s\n", sconf_strerror(&err));
    assert_int_equal(r, 0);

    const char *string = NULL;
    const int64_t *integer;
    const double *fp;
    const bool *boolean;

    /* Test string in array */
    r = sconf_get_str(root, "a.[0]", &string, NULL);
    assert_int_equal(r, 1);
    assert_string_equal(string, "foobar");

    /* Test integer in array */
    r = sconf_get_int(root, "a.[1]", &integer, NULL);
    assert_int_equal(r, 1);
    assert_int_equal(*integer, 42);

    /* Test floating-point number in array */
    r = sconf_get_float(root, "a.[2]", &fp, NULL);
    assert_int_equal(r, 1);
    assert_float_equal(*fp, 13.37, 0.0);

    /* Test boolean in array */
    r = sconf_get_bool(root, "a.[3]", &boolean, NULL);
    assert_int_equal(r, 1);
    assert_false(*boolean);

    /* Test dict -> array -> dict -> str */
    r = sconf_get_str(root, "b.a.[0].c", &string, NULL);
    assert_int_equal(r, 1);
    assert_string_equal(string, "rofl");

    /* Test dict -> array -> int */
    r = sconf_get_int(root, "b.a.[1]", &integer, NULL);
    assert_int_equal(r, 1);
    assert_int_equal(*integer, 2023);

    /* Test dict -> array -> dict -> dict -> str */
    r = sconf_get_str(root, "b.a.[2].e.f", &string, NULL);
    assert_int_equal(r, 1);
    assert_string_equal(string, "copter");

    /* Test dict -> array -> array -> int */
    r = sconf_get_int(root, "b.a.[3].[2]", &integer, NULL);
    assert_int_equal(r, 1);
    assert_int_equal(*integer, 100010101);

    /* Test dict -> array -> dict -> str */
    r = sconf_get_str(root, "b.a.[4].h", &string, NULL);
    assert_int_equal(r, 1);
    assert_string_equal(string, "123");

    sconf_node_destroy(root);
}

static void test_valid_yaml_multiple_documents(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_yaml_read(root, "yaml/test_multiple_documents.yaml", &err);
    printf("err: %s\n", sconf_strerror(&err));
    assert_int_equal(r, 0);

    const char *string = NULL;

    /* Test key from first document */
    r = sconf_get_str(root, "a", &string, NULL);
    assert_int_equal(r, 1);
    assert_string_equal(string, "b");

    /* Test key from second document */
    r = sconf_get_str(root, "c", &string, NULL);
    assert_int_equal(r, 1);
    assert_string_equal(string, "d");

    /* Test key from third document */
    r = sconf_get_str(root, "e", &string, NULL);
    assert_int_equal(r, 1);
    assert_string_equal(string, "f");

    sconf_node_destroy(root);
}

static void test_invalid_yaml_missing_file(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_yaml_read(root, "yaml/does-not-exist.yaml", &err);
    printf("err: %s\n", sconf_strerror(&err));
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_invalid_yaml_integer_overflow(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_yaml_read(root, "yaml/test_integer_overflow.yaml", &err);
    printf("err: %s\n", sconf_strerror(&err));
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_invalid_yaml_integer_underflow(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_yaml_read(root, "yaml/test_integer_underflow.yaml", &err);
    printf("err: %s\n", sconf_strerror(&err));
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_invalid_yaml_float_overflow(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_yaml_read(root, "yaml/test_float_overflow.yaml", &err);
    printf("err: %s\n", sconf_strerror(&err));
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_invalid_yaml_float_underflow(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_yaml_read(root, "yaml/test_float_underflow.yaml", &err);
    printf("err: %s\n", sconf_strerror(&err));
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_valid_yaml_empty_file(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_yaml_read(root, "yaml/test_empty.yaml", &err);
    printf("err: %s\n", sconf_strerror(&err));
    assert_int_equal(r, 0);

    sconf_node_destroy(root);
}

static void test_invalid_yaml_max_depth(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_yaml_read(root, "yaml/test_max_depth.yaml", &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_valid_yaml_string),
        cmocka_unit_test(test_valid_yaml_integer),
        cmocka_unit_test(test_valid_yaml_boolean),
        cmocka_unit_test(test_valid_yaml_float),
        cmocka_unit_test(test_valid_yaml_nested_dicts),
        cmocka_unit_test(test_valid_yaml_array),
        cmocka_unit_test(test_valid_yaml_multiple_documents),
        cmocka_unit_test(test_invalid_yaml_missing_file),
        cmocka_unit_test(test_invalid_yaml_integer_overflow),
        cmocka_unit_test(test_invalid_yaml_integer_underflow),
        cmocka_unit_test(test_invalid_yaml_integer_underflow),
        cmocka_unit_test(test_invalid_yaml_float_overflow),
        cmocka_unit_test(test_invalid_yaml_float_underflow),
        cmocka_unit_test(test_valid_yaml_empty_file),
        cmocka_unit_test(test_invalid_yaml_max_depth),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}

