#include <setjmp.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <cmocka.h>

#include "sconf.h"

void my_usage_callback(const char *usage, void *user)
{
    assert_non_null(usage);
}

static struct SConfMap default_map[] = {
    {
        .path = "bla.bla",
        .type = SCONF_TYPE_STR,
        .opts_short = 'a',
        .opts_long = "opt1",
        .help = "first option",
    },
    {
        .path = "foo.bar",
        .type = SCONF_TYPE_INT,
        .opts_short = 'b',
        .opts_long = "opt2",
        .arg_type = "something",
    },
    {
        .path = "rofl",
        .type = SCONF_TYPE_FLOAT,
        .opts_short = 'c',
        .opts_long = "opt3",
        .help = "third option",
    },
    {
        .path = "is.true",
        .type = SCONF_TYPE_BOOL,
        .opts_short = 'd',
        .opts_long = "opt4",
        .help = "oh no",
    },
    {
        .path = "config",
        .type = SCONF_TYPE_YAML_FILE,
        .opts_short = 'f',
        .opts_long = "config-file",
        .help = "read me",
    },
    {
        .type = SCONF_TYPE_USAGE,
        .opts_short = 'h',
        .opts_long = "help",
        .help = "print help",
        .usage_func = &my_usage_callback,
    },
    {0},
};

static void test_sconf_opts_parse_successful(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int argc = 10;
    char *argv[10] = { "test/test_sconf_opts_parse", "-a", "lol", "-b", "1337",
                       "--opt3", "13.37", "-d", "--config-file",
                       "yaml/test_integer.yaml" };

    int r = sconf_opts_parse(root, default_map, argc, argv, NULL, &err);
    assert_int_equal(r, 0);

    const char *string;
    r = sconf_get_str(root, "bla.bla", &string, &err);
    assert_int_equal(r, 1);
    assert_string_equal(string, "lol");

    const int64_t *integer;
    r = sconf_get_int(root, "foo.bar", &integer, &err);
    assert_int_equal(r, 1);
    assert_int_equal(*integer, 1337);

    const double *fp;
    r = sconf_get_float(root, "rofl", &fp, &err);
    assert_int_equal(r, 1);
    assert_float_equal(*fp, 13.37, 0.0);

    const bool *boolean;
    r = sconf_get_bool(root, "is.true", &boolean, &err);
    assert_int_equal(r, 1);
    assert_true(*boolean);

    r = sconf_get_str(root, "config", &string, &err);
    assert_int_equal(r, 1);
    assert_string_equal(string, "yaml/test_integer.yaml");

    r = sconf_get_int(root, "a", &integer, &err);
    assert_int_equal(r, 1);
    assert_int_equal(*integer, 1234567890);

    r = sconf_get_int(root, "e", &integer, &err);
    assert_int_equal(r, 1);
    assert_int_equal(*integer, -1234567890);

    sconf_node_destroy(root);
}

static void test_sconf_opts_parse_missing_root(void **unused)
{
    struct SConfErr err = {0};

    int argc = 1;
    char *argv[1] = { "test/test_sconf_opts_parse" };

    int r = sconf_opts_parse(NULL, default_map, argc, argv, NULL, &err);
    assert_int_equal(r, -1);
}

static void test_sconf_opts_parse_missing_map(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int argc = 1;
    char *argv[1] = { "test/test_sconf_opts_parse" };

    int r = sconf_opts_parse(root, NULL, argc, argv, NULL, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_opts_parse_no_options_in_map(void **unused)
{
    struct SConfMap map[] = {
        {
            .path = "aaa",
            .type = SCONF_TYPE_DICT,
        },
        {0},
    };

    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int argc = 3;
    char *argv[3] = { "test/test_sconf_opts_parse", "-a", "blabla" };

    int r = sconf_opts_parse(root, map, argc, argv, NULL, &err);
    assert_int_equal(r, 0);

    const char *string;
    r = sconf_get_str(root, "bla.bla", &string, &err);
    assert_int_equal(r, 0);

    sconf_node_destroy(root);
}

static void test_sconf_opts_parse_no_arguments(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int argc = 1;
    char *argv[1] = { "test/test_sconf_opts_parse" };

    int r = sconf_opts_parse(root, default_map, argc, argv, NULL, &err);
    assert_int_equal(r, 0);

    sconf_node_destroy(root);
}

static void test_sconf_opts_parse_option_missing_argument(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int argc = 2;
    char *argv[2] = { "test/test_sconf_opts_parse", "-a" };

    int r = sconf_opts_parse(root, default_map, argc, argv, NULL, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_opts_parse_unsupported_short_option(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int argc = 5;
    char *argv[5] = { "test/test_sconf_opts_parse", "-a", "abc", "-x", "foo" };

    int r = sconf_opts_parse(root, default_map, argc, argv, NULL, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_opts_parse_unsupported_long_option(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int argc = 5;
    char *argv[5] = { "test/test_sconf_opts_parse", "-b", "144", "--oh", "no" };

    int r = sconf_opts_parse(root, default_map, argc, argv, NULL, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_opts_parse_non_option_argument(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int argc = 6;
    char *argv[6] = { "test/test_sconf_opts_parse", "-b", "144", "145", "-a", "no" };

    int r = sconf_opts_parse(root, default_map, argc, argv, NULL, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_opts_parse_short_option_integer_wrong_type(void **unused)
{
    struct SConfMap map[] = {
        {
            .path = "aaa",
            .type = SCONF_TYPE_INT,
            .opts_short = 'z',
        },
        {0},
    };

    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int argc = 3;
    char *argv[3] = { "test/test_sconf_opts_parse", "-z", "true" };

    int r = sconf_opts_parse(root, map, argc, argv, NULL, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_opts_parse_long_option_integer_wrong_type(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int argc = 3;
    char *argv[3] = { "test/test_sconf_opts_parse", "--opt2", "lol" };

    int r = sconf_opts_parse(root, default_map, argc, argv, NULL, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_opts_parse_short_option_float_wrong_type(void **unused)
{
    struct SConfMap map[] = {
        {
            .path = "nnn",
            .type = SCONF_TYPE_FLOAT,
            .opts_short = 'n',
        },
        {0},
    };

    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int argc = 3;
    char *argv[3] = { "test/test_sconf_opts_parse", "-n", "hello" };

    int r = sconf_opts_parse(root, map, argc, argv, NULL, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_opts_parse_long_option_float_wrong_type(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int argc = 3;
    char *argv[3] = { "test/test_sconf_opts_parse", "--opt3", "no" };

    int r = sconf_opts_parse(root, default_map, argc, argv, NULL, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_opts_parse_short_option_boolean_wrong_type(void **unused)
{
    struct SConfMap map[] = {
        {
            .path = "qqq",
            .type = SCONF_TYPE_BOOL,
            .opts_short = 'q',
        },
        {0},
    };

    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int argc = 2;
    char *argv[2] = { "test/test_sconf_opts_parse", "-q3.14" };

    int r = sconf_opts_parse(root, map, argc, argv, NULL, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_opts_parse_long_option_boolean_wrong_type(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int argc = 2;
    char *argv[2] = { "test/test_sconf_opts_parse", "--opt4=aaa" };

    int r = sconf_opts_parse(root, default_map, argc, argv, NULL, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_opts_parse_non_existing_yaml_file(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int argc = 3;
    char *argv[3] = { "test/test_sconf_opts_parse", "--config", "does-not-exist" };

    int r = sconf_opts_parse(root, default_map, argc, argv, NULL, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_opts_parse_unsupported_type(void **unused)
{
    struct SConfMap map[] = {
        {
            .path = "b",
            .type = SCONF_TYPE_DICT,
            .opts_short = 'b',
            .opts_long = "bbb",
        },
        {0},
    };

    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int argc = 3;
    char *argv[3] = { "test/test_sconf_opts_parse", "--bbb", "foo" };

    int r = sconf_opts_parse(root, map, argc, argv, NULL, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_opts_parse_usage(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int argc = 2;
    char *argv[2] = { "test/test_sconf_opts_parse", "-h" };

    int r = sconf_opts_parse(root, default_map, argc, argv, NULL, &err);
    assert_int_equal(r, 0);

    sconf_node_destroy(root);
}

static void test_sconf_opts_parse_short_option_duplicates(void **unused)
{
    struct SConfMap map[] = {
        {
            .path = "b",
            .type = SCONF_TYPE_STR,
            .opts_short = 'b',
            .opts_long = "bbb",
        },
        {
            .path = "c",
            .type = SCONF_TYPE_STR,
            .opts_short = 'b',
        },
        {0},
    };

    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int argc = 3;
    char *argv[3] = { "test/test_sconf_opts_parse", "-b", "foo" };

    int r = sconf_opts_parse(root, map, argc, argv, NULL, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_opts_parse_missing_path(void **unused)
{
    struct SConfMap map[] = {
        {
            .type = SCONF_TYPE_STR,
            .opts_short = 'b',
            .opts_long = "bbb",
        },
        {0},
    };

    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int argc = 3;
    char *argv[3] = { "test/test_sconf_opts_parse", "-b", "foobar" };

    int r = sconf_opts_parse(root, map, argc, argv, NULL, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

static void test_sconf_opts_parse_missing_short_option(void **unused)
{
    struct SConfMap map[] = {
        {
            .path = "hmz",
            .type = SCONF_TYPE_INT,
            .opts_long = "xyx",
        },
        {0},
    };

    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int argc = 3;
    char *argv[3] = { "test/test_sconf_opts_parse", "--xyx", "123" };

    int r = sconf_opts_parse(root, map, argc, argv, NULL, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_sconf_opts_parse_successful),
        cmocka_unit_test(test_sconf_opts_parse_missing_root),
        cmocka_unit_test(test_sconf_opts_parse_missing_map),
        cmocka_unit_test(test_sconf_opts_parse_no_options_in_map),
        cmocka_unit_test(test_sconf_opts_parse_no_arguments),
        cmocka_unit_test(test_sconf_opts_parse_option_missing_argument),
        cmocka_unit_test(test_sconf_opts_parse_unsupported_short_option),
        cmocka_unit_test(test_sconf_opts_parse_unsupported_long_option),
        cmocka_unit_test(test_sconf_opts_parse_non_option_argument),
        cmocka_unit_test(test_sconf_opts_parse_short_option_integer_wrong_type),
        cmocka_unit_test(test_sconf_opts_parse_long_option_integer_wrong_type),
        cmocka_unit_test(test_sconf_opts_parse_short_option_float_wrong_type),
        cmocka_unit_test(test_sconf_opts_parse_long_option_float_wrong_type),
        cmocka_unit_test(test_sconf_opts_parse_short_option_boolean_wrong_type),
        cmocka_unit_test(test_sconf_opts_parse_long_option_boolean_wrong_type),
        cmocka_unit_test(test_sconf_opts_parse_non_existing_yaml_file),
        cmocka_unit_test(test_sconf_opts_parse_unsupported_type),
        cmocka_unit_test(test_sconf_opts_parse_usage),
        cmocka_unit_test(test_sconf_opts_parse_short_option_duplicates),
        cmocka_unit_test(test_sconf_opts_parse_missing_path),
        cmocka_unit_test(test_sconf_opts_parse_missing_short_option),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}

