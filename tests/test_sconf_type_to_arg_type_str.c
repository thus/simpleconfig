#include <setjmp.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <cmocka.h>

#include "sconf_private.h"

static void test_sconf_arg_type_strings(void **unused)
{
    assert_string_equal(sconf_type_to_arg_type_str(SCONF_TYPE_STR), "<str>");
    assert_string_equal(sconf_type_to_arg_type_str(SCONF_TYPE_INT), "<int>");
    assert_string_equal(sconf_type_to_arg_type_str(SCONF_TYPE_FLOAT), "<float>");
    assert_string_equal(sconf_type_to_arg_type_str(SCONF_TYPE_BOOL), "");
    assert_string_equal(sconf_type_to_arg_type_str(SCONF_TYPE_YAML_FILE), "<file>");
    assert_string_equal(sconf_type_to_arg_type_str(SCONF_TYPE_USAGE), "");
}

static void test_sconf_type_to_arg_type_str_out_of_bounds(void **unused)
{
    assert_string_equal(sconf_type_to_arg_type_str(SCONF_TYPE_MAX + 1), "TYPE NOT USED FOR OPTIONS");
    assert_string_equal(sconf_type_to_arg_type_str(200), "TYPE NOT USED FOR OPTIONS");
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_sconf_arg_type_strings),
        cmocka_unit_test(test_sconf_type_to_arg_type_str_out_of_bounds),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}

