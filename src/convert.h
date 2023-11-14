#pragma once

#include "sconf.h"

int sconf_string_to_integer(const char *, int64_t *, struct SConfErr *);
int sconf_string_to_float(const char *, double *, struct SConfErr *);
int sconf_string_to_bool(const char *, bool *);

