#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "sconf.h"

#define NUMBER_BASE_OCTAL       8
#define NUMBER_BASE_DECIMAL     10
#define NUMBER_BASE_HEXADECIMAL 16

/**
 * @brief Convert string to integer.
 *
 * @param string  String to convert.
 * @param integer Pointer to integer to store result in.
 *
 * @return 1 if string is integer, 0 if not, or -1 on error.
 */
int sconf_string_to_integer(const char *string, int64_t *integer,
                            struct SConfErr *err)
{
    errno = 0;
    int base = NUMBER_BASE_DECIMAL;

    if ((string[0] == '0') && (string[1] == 'x')) {
        base = NUMBER_BASE_HEXADECIMAL;
    }
    else if ((string[0] == '0') && (strlen(string) > 1)) {
        base = NUMBER_BASE_OCTAL;
    }

    char *endptr;
    *integer = strtoll(string, &endptr, base);
    if (*endptr != '\0') {
        return 0;
    }

    if (errno == ERANGE) {
        if (*integer == LLONG_MAX) {
            sconf_err_set(err, "integer value overflow detected");
        }
        else {
            sconf_err_set(err, "integer value underflow detected");
        }
        return -1;
    }

    return 1;
}

/**
 * @brief Convert string to floating point number.
 *
 * @param string String to convert.
 * @param fp     Pointer to floating point number to store result in.
 * @param err    Pointer to error struct.
 *
 * @return 1 if string is floating point number, 0 if not, or -1 on error.
 */
int sconf_string_to_float(const char *string, double *fp, struct SConfErr *err)
{
    errno = 0;

    char *endptr;
    *fp = strtod(string, &endptr);
    if (*endptr != '\0') {
        return 0;
    }

    if (errno == ERANGE) {
        if (*fp == HUGE_VAL) {
            sconf_err_set(err, "floating-point number overflow detected");
        }
        else {
            sconf_err_set(err, "floating-point number underflow detected");
        }
        return -1;
    }

    return 1;
}

/**
 * @internal
 * @brief Convert string to boolean.
 *
 * @param string  String to convert.
 * @param boolean Pointer to boolean to store result in.
 *
 * @return 1 if string is boolean, 0 if not, or -1 on error.
 */
int sconf_string_to_bool(const char *string, bool *boolean)
{
    if (strncmp(string, "true", 4) == 0) {
        *boolean = true;
        return 1;
    }
    else if (strncmp(string, "false", 5) == 0) {
        *boolean = false;
        return 1;
    }
    else if (strncmp(string, "yes", 3) == 0) {
        *boolean = true;
        return 1;
    }
    else if (strncmp(string, "no", 2) == 0) {
        *boolean = false;
        return 1;
    }
    else if (strncmp(string, "on", 2) == 0) {
        *boolean = true;
        return 1;
    }
    else if (strncmp(string, "off", 3) == 0) {
        *boolean = false;
        return 1;
    }

    return 0;
}

