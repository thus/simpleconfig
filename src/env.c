#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "convert.h"
#include "sconf_private.h"

/**
 * @internal
 * @brief Set config string from environment variable.
 *
 * @param root  The config root node.
 * @param path  Path to config node.
 * @param value Value of environment variable.
 * @param err   Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
static int sconf_env_set_str(struct SConfNode *root, const char *path,
                             const char *value, struct SConfErr *err)
{
    assert(root);
    assert(path);
    assert(value);

    if (sconf_set_str(root, path, value, err) == -1) {
        return -1;
    }

    return 0;
}

/**
 * @internal
 * @brief Set config integer from environment variable.
 *
 * @param root  The config root node.
 * @param path  Path to config node.
 * @param env   Name of environment variable.
 * @param value Value of environment variable.
 * @param err   Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
static int sconf_env_set_int(struct SConfNode *root, const char *path,
                             const char *env, const char *value,
                             struct SConfErr *err)
{
    assert(root);
    assert(path);
    assert(env);
    assert(value);

    int64_t integer;

    int r = sconf_string_to_integer(value, &integer, err);
    if (r == -1) {
        return -1;
    }
    if (r == 0) {
        sconf_err_set(err, "expected integer for environment variable %s",
                      env);
        return -1;
    }

    if (sconf_set_int(root, path, integer, err) == -1) {
        return -1;
    }

    return 0;
}

/**
 * @internal
 * @brief Set config floating-point number from environment variable.
 *
 * @param root  The config root node.
 * @param path  Path to config node.
 * @param env   Name of environment variable.
 * @param value Value of environment variable.
 * @param err   Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
static int sconf_env_set_float(struct SConfNode *root, const char *path,
                               const char *env, const char *value,
                               struct SConfErr *err)
{
    assert(root);
    assert(path);
    assert(env);
    assert(value);

    double fp;

    int r = sconf_string_to_float(value, &fp, err);
    if (r == -1) {
        return -1;
    }
    if (r == 0) {
        sconf_err_set(err, "expected floating-point number for environment "
                      "variable %s", env);
        return -1;
    }

    if (sconf_set_float(root, path, fp, err) == -1) {
        return -1;
    }

    return 0;
}

/**
 * @internal
 * @brief Set config boolean from environment variable.
 *
 * @param root  The config root node.
 * @param path  Path to config node.
 * @param env   Name of environment variable.
 * @param value Value of environment variable.
 * @param err   Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
static int sconf_env_set_bool(struct SConfNode *root, const char *path,
                              const char *env, const char *value,
                              struct SConfErr *err)
{
    assert(root);
    assert(path);
    assert(env);
    assert(value);

    bool boolean;

    int r = sconf_string_to_bool(value, &boolean);
    if (r == 0) {
        sconf_err_set(err, "expected boolean for environment variable %s", env);
        return -1;
    }

    if (sconf_set_bool(root, path, boolean, err) == -1) {
        return -1;
    }

    return 0;
}

/**
 * @internal
 * @brief Read YAML file from environment variable.
 *
 * @param root  The config root node.
 * @param path  Path to config node.
 * @param value Value of environment variable.
 * @param err   Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
static int sconf_env_yaml_file(struct SConfNode *root, const char *path,
                               const char *value, struct SConfErr *err)
{
    assert(root);
    assert(path);
    assert(value);

    if (sconf_yaml_read(root, value, err) == -1) {
        return -1;
    }
    if (sconf_set_str(root, path, value, err) == -1) {
        return -1;
    }

    return 0;
}

/**
 * @brief Read environment variables based on config map.
 *
 * @param root The config root node.
 * @param map  Config map.
 * @param err  Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
int sconf_env_read(struct SConfNode *root, const struct SConfMap *map,
                   struct SConfErr *err)
{
    if (!root) {
        sconf_err_set(err, "no root specified when reading env");
        return -1;
    }

    if (!map) {
        sconf_err_set(err, "no map specified when reading env");
        return -1;
    }

    for (const struct SConfMap *entry = map; entry->type; entry++)
    {
        if (!entry->env) {
            /* No environment variable defined for entry */
            continue;
        }

        if (!entry->path) {
            sconf_err_set(err, "must have 'path' specified to use 'env'");
            return -1;
        }

        char *value = getenv(entry->env);
        if (!value) {
            continue;
        }

        switch (entry->type)
        {
            case SCONF_TYPE_STR:
                if (sconf_env_set_str(root, entry->path, value, err) == -1) {
                    return -1;
                }
                break;

            case SCONF_TYPE_INT:
                if (sconf_env_set_int(root, entry->path, entry->env, value,
                                      err) == -1) {
                    return -1;
                }
                break;

            case SCONF_TYPE_FLOAT:
                if (sconf_env_set_float(root, entry->path, entry->env, value,
                                        err) == -1) {
                    return -1;
                }
                break;

            case SCONF_TYPE_BOOL:
                if (sconf_env_set_bool(root, entry->path, entry->env, value,
                                       err) == -1) {
                    return -1;
                }
                break;

            case SCONF_TYPE_YAML_FILE:
                if (sconf_env_yaml_file(root, entry->path, value, err) == -1) {
                    return -1;
                }
                break;

            default:
                sconf_err_set(err, "type %s cannot be used for reading env",
                              sconf_type_to_str(entry->type));
                return -1;
        }
    }

    return 0;
}

