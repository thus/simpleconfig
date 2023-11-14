#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>

#include "array.h"
#include "sconf.h"

#define SCONF_ARRAY_MAX_SIZE 65536

/**
 * @brief Create dynamic array.
 *
 * @param size Initial size of array.
 * @param err  Pointer to error struct.
 *
 * @return array on success, NULL otherwise.
 */
struct SConfArray *sconf_array_create(uint32_t size, struct SConfErr *err)
{
    if (size == 0) {
        sconf_err_set(err, "array size must be >0");
        return NULL;
    }

    if (size > SCONF_ARRAY_MAX_SIZE) {
        sconf_err_set(err, "array size must be <%d", SCONF_ARRAY_MAX_SIZE);
        return NULL;
    }

    struct SConfArray *array = calloc(1, sizeof(struct SConfArray));
    if (!array) {
        sconf_err_set(err, "failed to allocate memory for array");
        return NULL;
    }

    array->entries = calloc(size, sizeof(struct SConfNode *));
    if (!array->entries) {
        sconf_err_set(err, "failed to allocate memory for array elements");
        free(array);
        return NULL;
    }

    array->size = size;

    return array;
}

/**
 * @brief Destroy dynamic array.
 *
 * @param array Array to destroy.
 */
void sconf_array_destroy(struct SConfArray *array)
{
    if (!array) {
        return;
    }

    if (array->entries) {
        free(array->entries);
    }

    free(array);
}

/**
 * @internal
 * @brief Grow array to make room if necessary.
 *
 * @param array       Array to check.
 * @param size_needed How much room the array must have.
 * @param err         Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
static int sconf_array_grow(struct SConfArray *array, uint32_t size_needed,
                            struct SConfErr *err)
{
    assert(array);
    assert(array->entries);

    if (array->size >= size_needed) {
        return 0;
    }

    if (size_needed > SCONF_ARRAY_MAX_SIZE) {
        sconf_err_set(err, "array is full (max size reached)");
        return -1;
    }

    struct SConfNode **new = realloc(array->entries,
                                     sizeof(struct SConfNode *) * size_needed);
    if (!new) {
        sconf_err_set(err, "failed to realloc array");
        return -1;
    }

    for (uint32_t i = array->size; i < size_needed; i++)
    {
        new[i] = NULL;
    }

    array->entries = new;
    array->size = size_needed;

    return 0;
}

/**
 * @brief Insert into dynamic array.
 *
 * @param array Array to insert into.
 * @param index Where in array to insert.
 * @param node  Node to insert.
 *
 * @return 0 on success, -1 otherwise.
 */
int sconf_array_insert(struct SConfArray *array, uint32_t index,
                       struct SConfNode *node, struct SConfErr *err)
{
    if (!array || !array->entries) {
        sconf_err_set(err, "array must be defined");
        return -1;
    }

    if (!node) {
        sconf_err_set(err, "node to insert must be defined");
        return -1;
    }

    if (sconf_array_grow(array, index + 1, err) != 0) {
        return -1;
    }

    if (array->entries[index]) {
        sconf_err_set(err, "there is already a node in array at index '%"
                      PRIu32 "'", index);
        return -1;
    }

    array->entries[index] = node;

    return 0;
}

