#include <assert.h>
#include <stdint.h>

#include "convert.h"
#include "sconf_private.h"

/**
 * @internal
 * @brief Apply default string.
 *
 * @param root The config root node.
 * @param curr Config node entry.
 * @param err  Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
static int sconf_defaults_handle_str(struct SConfNode *root,
                                     const struct SConfMap *curr,
                                     struct SConfErr *err)
{
    assert(root);
    assert(curr);
    assert(curr->path);
    assert(curr->default_value);

    const char *string;
    int r = sconf_get_str(root, curr->path, &string, err);
    if (r == -1) {
        return -1;
    }
    else if (r == 1) {
        /* String is already set */
        return 0;
    }

    if (sconf_set_str(root, curr->path, curr->default_value, err) == -1) {
        return -1;
    }

    return 0;
}

/**
 * @internal
 * @brief Apply default integer.
 *
 * @param root The config root node.
 * @param curr Config node entry.
 * @param err  Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
static int sconf_defaults_handle_int(struct SConfNode *root,
                                     const struct SConfMap *curr,
                                     struct SConfErr *err)
{
    assert(root);
    assert(curr);
    assert(curr->path);
    assert(curr->default_value);

    struct SConfNode *node;
    int r = sconf_get(root, curr->path, &node, err);
    if (r == -1) {
        return -1;
    }
    else if (r == 1) {
        /* A node with this path already exists */
        return 0;
    }

    int64_t integer;
    r = sconf_string_to_integer(curr->default_value, &integer, err);
    if (r == -1) {
        return -1;
    }
    else if (r == 0) {
        sconf_err_set(err, "expected default value for '%s' to be integer",
                      curr->path);
        return -1;
    }

    if (sconf_set_int(root, curr->path, integer, err) == -1) {
        return -1;
    }

    return 0;
}

/**
 * @internal
 * @brief Apply default floating-point number.
 *
 * @param root The config root node.
 * @param curr Config node entry.
 * @param err  Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
static int sconf_defaults_handle_float(struct SConfNode *root,
                                       const struct SConfMap *curr,
                                       struct SConfErr *err)
{
    assert(root);
    assert(curr);
    assert(curr->path);
    assert(curr->default_value);

    struct SConfNode *node;
    int r = sconf_get(root, curr->path, &node, err);
    if (r == -1) {
        return -1;
    }
    else if (r == 1) {
        /* A node with this path already exists */
        return 0;
    }

    double fp;
    r = sconf_string_to_float(curr->default_value, &fp, err);
    if (r == -1) {
        return -1;
    }
    else if (r == 0) {
        sconf_err_set(err, "expected default value for '%s' to be "
                      "floating-point number", curr->path);
        return -1;
    }

    if (sconf_set_float(root, curr->path, fp, err) == -1) {
        return -1;
    }

    return 0;
}

/**
 * @internal
 * @brief Apply default boolean.
 *
 * @param root The config root node.
 * @param curr Config node entry.
 * @param err  Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
static int sconf_defaults_handle_bool(struct SConfNode *root,
                                      const struct SConfMap *curr,
                                      struct SConfErr *err)
{
    assert(root);
    assert(curr);
    assert(curr->path);
    assert(curr->default_value);

    struct SConfNode *node;
    int r = sconf_get(root, curr->path, &node, err);
    if (r == -1) {
        return -1;
    }
    else if (r == 1) {
        /* A node with this path already exists */
        return 0;
    }

    bool boolean;
    r = sconf_string_to_bool(curr->default_value, &boolean);
    if (r == -1) {
        return -1;
    }
    else if (r == 0) {
        sconf_err_set(err, "expected default value for '%s' to be boolean",
                      curr->path);
        return -1;
    }

    if (sconf_set_bool(root, curr->path, boolean, err) == -1) {
        return -1;
    }

    return 0;
}

/**
 * @internal
 * @brief Apply default YAML file.
 *
 * @param root The config root node.
 * @param curr Config node entry.
 * @param err  Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
static int sconf_defaults_handle_yaml_file(struct SConfNode *root,
                                           const struct SConfMap *curr,
                                           struct SConfErr *err)
{
    assert(root);
    assert(curr);
    assert(curr->path);
    assert(curr->default_value);

    const char *string;
    int r = sconf_get_str(root, curr->path, &string, err);
    if (r == -1) {
        return -1;
    }
    else if (r == 1) {
        /* YAML file path is already set */
        return 0;
    }

    if (sconf_yaml_read(root, curr->default_value, err) == -1) {
        return -1;
    }
    if (sconf_set_str(root, curr->path, curr->default_value, err) == -1) {
        return -1;
    }

    return 0;
}

/**
 * @brief Apply configuration defaults.
 *
 * @param root The config root node.
 * @param map  Config map.
 * @param err  Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
int sconf_defaults(struct SConfNode *root, const struct SConfMap *map,
                   struct SConfErr *err)
{
    if (!root) {
        sconf_err_set(err, "no root specified when applying defaults");
        return -1;
    }

    if (!map) {
        sconf_err_set(err, "no map specified when applying defaults");
        return -1;
    }

    for (const struct SConfMap *entry = map; entry->type; entry++)
    {
        if (!entry->default_value) {
            /* No default value configured for entry */
            continue;
        }

        if (!entry->path) {
            sconf_err_set(err, "config entry map is missing path");
            return -1;
        }

        switch (entry->type)
        {
            case SCONF_TYPE_STR:
                if (sconf_defaults_handle_str(root, entry, err) == -1) {
                    return -1;
                }
                break;

            case SCONF_TYPE_INT:
                if (sconf_defaults_handle_int(root, entry, err) == -1) {
                    return -1;
                }
                break;

            case SCONF_TYPE_FLOAT:
                if (sconf_defaults_handle_float(root, entry, err) == -1) {
                    return -1;
                }
                break;

            case SCONF_TYPE_BOOL:
                if (sconf_defaults_handle_bool(root, entry, err) == -1) {
                    return -1;
                }
                break;

            case SCONF_TYPE_YAML_FILE:
                if (sconf_defaults_handle_yaml_file(root, entry, err) == -1) {
                    return -1;
                }
                break;

            default:
                sconf_err_set(err, "type %s cannot be used for defaults",
                              sconf_type_to_str(entry->type));
                return -1;
        }
    }

    return 0;
}

