#pragma once

#include <stdint.h>

#include "sconf.h"

struct SConfArray {
    struct SConfNode **entries;
    uint32_t size;
};

struct SConfArray *sconf_array_create(uint32_t size, struct SConfErr *err);
void sconf_array_destroy(struct SConfArray *array);
int sconf_array_insert(struct SConfArray *array, uint32_t index,
                       struct SConfNode *node, struct SConfErr *err);

