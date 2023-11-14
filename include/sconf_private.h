#pragma once

#include "art.h"

#include "sconf.h"

/**
 * Private structure representing a config node. Should not be used
 * directly outside the library.
 */
struct SConfNode {
    uint8_t type;

    union {
        art_tree dictionary;
        char *string;
        int64_t integer;
        bool boolean;
        double fp;
        struct SConfArray *array;
    };
};

/**
 * Return string representation of type.
 */
const char *sconf_type_to_str(uint8_t type);

/**
 * Return string representation of argument type.
 */
const char *sconf_type_to_arg_type_str(uint8_t arg_type);

