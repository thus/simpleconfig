#include <assert.h>
#include <stdbool.h>

#include "sconf_private.h"

/**
 * @internal
 * @brief Check if config requirements are met.
 *
 * @param root The config root node.
 * @param curr Config map entry to check.
 * @param err  Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
static int sconf_validate_check_required(struct SConfNode *root,
                                         const struct SConfMap *curr,
                                         struct SConfErr *err)
{
    assert(root);
    assert(curr);

    if (!curr->required) {
        return 0;
    }

    if (!curr->path) {
        sconf_err_set(err, "must have 'path' specified to use 'required'");
        return -1;
    }

    struct SConfNode *node = NULL;
    int r = sconf_get(root, curr->path, &node, err);
    if (r == -1) {
        return -1;
    }
    else if (r == 0) {
        sconf_err_set(err, "required config path '%s' does not exist",
                      curr->path);
        return -1;
    }

    if (curr->type != sconf_type(node)) {
        sconf_err_set(err, "required config path '%s' exists, but is wrong "
                           "type %s != %s", curr->path,
                           sconf_type_to_str(sconf_type(node)),
                           sconf_type_to_str(curr->type));
        return -1;
    }

    return 0;
}

/**
 * @internal
 * @brief Run config validate function.
 *
 * @param root The config root node.
 * @param curr Config map entry.
 * @param user User-supplied data passed to validate callback functions.
 * @param err  Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
static int sconf_validate_run_validate_func(struct SConfNode *root,
                                            const struct SConfMap *curr,
                                            void *user, struct SConfErr *err)
{
    assert(root);
    assert(curr);

    if (!curr->validate_func) {
        return 0;
    }

    if (!curr->path) {
        sconf_err_set(err, "must have 'path' specified to use 'validate_func'");
        return -1;
    }

    struct SConfNode *node = NULL;
    int r = sconf_get(root, curr->path, &node, err);
    if (r == -1) {
        return -1;
    }

    r = curr->validate_func(curr->path, node, user, err);
    if (r != 0) {
        return -1;
    }

    return 0;
}

/**
 * @brief Validate config based on config map.
 *
 * @param root The config root node.
 * @param map  Config map.
 * @param user User-supplied data passed to validate callback functions.
 * @param err  Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
int sconf_validate(struct SConfNode *root, const struct SConfMap *map,
                   void *user, struct SConfErr *err)
{
    if (!root) {
        sconf_err_set(err, "no root specified when validating config");
        return -1;
    }

    if (!map) {
        sconf_err_set(err, "no map specified when validating config");
        return -1;
    }

    for (const struct SConfMap *entry = map; entry->type; entry++)
    {
        if (sconf_validate_check_required(root, entry, err) == -1) {
            return -1;
        }

        if (sconf_validate_run_validate_func(root, entry, user, err) == -1) {
            return -1;
        }
    }

    return 0;
}

