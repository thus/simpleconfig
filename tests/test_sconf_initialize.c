#include <setjmp.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <cmocka.h>

#include "sconf.h"

static struct SConfMap default_map[] = {
    {
        .path = "a",
        .type = SCONF_TYPE_STR,
        .opts_short = 'a',
        .opts_long = "aaa",
        .help = "a a aaa a a aaa",
        .arg_type = "A",
        .env = "A",
        .default_value = "a",
    },
    {0}
};

static void test_sconf_initialize_successful(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_initialize(root, default_map, 0, NULL, NULL, &err);
    assert_int_equal(r, 0);

    sconf_node_destroy(root);
}

static void test_sconf_initialize_missing_root(void **unused)
{
    struct SConfErr err = {0};

    int r = sconf_initialize(NULL, default_map, 0, NULL, NULL, &err);
    assert_int_equal(r, -1);
}

static void test_sconf_initialize_missing_map(void **unused)
{
    struct SConfNode *root = sconf_node_create(SCONF_TYPE_DICT, NULL, NULL);
    assert_non_null(root);

    struct SConfErr err = {0};

    int r = sconf_initialize(root, NULL, 0, NULL, NULL, &err);
    assert_int_equal(r, -1);

    sconf_node_destroy(root);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_sconf_initialize_successful),
        cmocka_unit_test(test_sconf_initialize_missing_root),
        cmocka_unit_test(test_sconf_initialize_missing_map),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}

