#include <setjmp.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <cmocka.h>

#include "sconf.h"

static void test_sconf_err_set(void **unused)
{
    struct SConfErr err;
    const char *string = "foo bar";
    const int num = 1234567890;
    sconf_err_set(&err, "msg: %s (%d)", string, num);
    assert_string_equal(sconf_strerror(&err), "msg: foo bar (1234567890)");
}

static void test_sconf_err_set_return_on_null(void **unused)
{
    sconf_err_set(NULL, "foo");
}

static void test_sconf_err_set_too_long_msg(void **unused)
{
    const char *too_long_err_msg =
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz"
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz"
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz"
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz"
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuv";

    struct SConfErr err;
    sconf_err_set(&err, "%s", too_long_err_msg);
    assert_string_equal(sconf_strerror(&err), "setting error message failed");
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_sconf_err_set),
        cmocka_unit_test(test_sconf_err_set_return_on_null),
        cmocka_unit_test(test_sconf_err_set_too_long_msg),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}

