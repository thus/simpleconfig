#include <setjmp.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <cmocka.h>

#include "sconf_private.h"

static void test_sconf_type_strings(void **unused)
{
    assert_string_equal(sconf_type_to_str(SCONF_TYPE_UNKNOWN), "unknown");
    assert_string_equal(sconf_type_to_str(SCONF_TYPE_DICT), "dictionary");
    assert_string_equal(sconf_type_to_str(SCONF_TYPE_ARRAY), "array");
    assert_string_equal(sconf_type_to_str(SCONF_TYPE_STR), "string");
    assert_string_equal(sconf_type_to_str(SCONF_TYPE_INT), "integer");
    assert_string_equal(sconf_type_to_str(SCONF_TYPE_BOOL), "boolean");
    assert_string_equal(sconf_type_to_str(SCONF_TYPE_FLOAT), "floating-point number");
    assert_string_equal(sconf_type_to_str(SCONF_TYPE_YAML_FILE), "YAML file");
    assert_string_equal(sconf_type_to_str(SCONF_TYPE_USAGE), "usage");
    assert_string_equal(sconf_type_to_str(SCONF_TYPE_MAX), "not-used");
}

static void test_sconf_type_to_str_out_of_bounds(void **unused)
{
    assert_string_equal(sconf_type_to_str(SCONF_TYPE_MAX + 1), "not-used");
    assert_string_equal(sconf_type_to_str(200), "not-used");
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_sconf_type_strings),
        cmocka_unit_test(test_sconf_type_to_str_out_of_bounds),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}

