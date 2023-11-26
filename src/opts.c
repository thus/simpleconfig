#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "convert.h"
#include "sconf_private.h"

/* Size of array containing long options. All long options is linked up
   with short options, which limits the size of available options. */
#define SCONF_OPTS_LONG_OPTS_SIZE (CHAR_MAX + 1)

/* Size of options index. All long options is linked up with short
   options, which limits the size of available options. */
#define SCONF_OPTS_INDEX_SIZE (CHAR_MAX + 1)

/* Size of optstring containing all possible short options and arguments,
   optional and not. */
#define SCONF_OPTS_OPTSTRING_MAX_SIZE 256

/* Maximum size of usage string */
#define SCONF_OPTS_USAGE_STRING_MAX 4096

/**
 * @internal
 * @brief Create index mapping short opts to config map entries.
 *
 * @param map   Config map.
 * @param index Index of config map entries.
 * @param err   Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
static int sconf_short_opts_index_create(const struct SConfMap *map,
                                         const struct SConfMap **index,
                                         struct SConfErr *err)
{
    assert(map);
    assert(index);

    for (const struct SConfMap *entry = map; entry->type; entry++)
    {
        if (!entry->opts_short) {
            continue;
        }

        if (entry->opts_short < 0) {
            sconf_err_set(err, "short option '%c' is a negative number",
                          entry->opts_short);
            return -1;
        }

        unsigned char opt_index = entry->opts_short;

        if (index[opt_index]) {
            sconf_err_set(err, "short option '%c' is used more than once",
                          entry->opts_short);
            return -1;
        }

        /* Path is required, unless type is 'usage' */
        if (!entry->path && entry->type != SCONF_TYPE_USAGE) {
            sconf_err_set(err, "path is missing for short option '%c'",
                          entry->opts_short);
            return -1;
        }

        index[opt_index] = entry;
    }

    return 0;
}

/**
 * @internal
 * @brief Create long options from config map.
 *
 * @param map       Config map.
 * @param long_opts Long option struct to set.
 * @param err       Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
static int sconf_opts_create_long_opts(const struct SConfMap *map,
                                       struct option *long_opts,
                                       struct SConfErr *err)
{
    assert(map);
    assert(long_opts);

    int i = 0;

    for (const struct SConfMap *entry = map; entry->type; entry++)
    {
        if (!entry->opts_long) {
            continue;
        }

        if (!entry->opts_short) {
            sconf_err_set(err, "long option '%s' has no short option",
                          entry->opts_long);
            return -1;
        }

        if (i >= SCONF_OPTS_LONG_OPTS_SIZE) {
            sconf_err_set(err, "long option array is full");
            return -1;
        }

        long_opts[i].name = entry->opts_long;
        long_opts[i].val = (unsigned char)entry->opts_short;

        switch (entry->type)
        {
            case SCONF_TYPE_STR:
                /* Fall through */
            case SCONF_TYPE_INT:
                /* Fall through */
            case SCONF_TYPE_FLOAT:
                /* Fall through */
            case SCONF_TYPE_YAML_FILE:
                long_opts[i].has_arg = required_argument;
                break;
            case SCONF_TYPE_BOOL:
                long_opts[i].has_arg = optional_argument;
                break;
            case SCONF_TYPE_USAGE:
                long_opts[i].has_arg = no_argument;
                break;
            default:
                sconf_err_set(err, "unsupported config node type '%s' used in "
                              "config map", sconf_type_to_str(entry->type));
                return -1;
        }

        i++;
    }

    return 0;
}

/**
 * @internal
 * @brief Create optstring from config map.
 *
 * @param map       Config map.
 * @param optstring Pointer to optstring.
 * @param err       Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
static int sconf_opts_create_optstring(const struct SConfMap *map,
                                       char *optstring, struct SConfErr *err)
{
    assert(map);
    assert(optstring);

    int pos = 1;

    /* Used to get `getopt_long` to return ':' if option argument is
       missing, and '?' if an unsupported option is specified. */
    optstring[0] = ':';

    for (const struct SConfMap *entry = map; entry->type; entry++)
    {
        if (!entry->opts_short) {
            continue;
        }

        /* Make sure there are room for more options in string */
        if (pos + 3 >= SCONF_OPTS_OPTSTRING_MAX_SIZE - 1) {
            sconf_err_set(err, "optstring reached max length");
            return -1;
        }

        optstring[pos] = entry->opts_short;
        pos++;

        /* These config types has argument */
        switch (entry->type)
        {
            case SCONF_TYPE_STR:
                /* Fall through */
            case SCONF_TYPE_INT:
                /* Fall through */
            case SCONF_TYPE_FLOAT:
                /* Fall through */
            case SCONF_TYPE_YAML_FILE:
                /* Required argument */
                optstring[pos] = ':';
                pos++;
                break;
            case SCONF_TYPE_BOOL:
                /* Optional argument */
                optstring[pos] = ':';
                pos++;
                optstring[pos] = ':';
                pos++;
                break;
        }
    }

    return 0;
}

/**
 * @internal
 * @brief Handle string option.
 *
 * @param root  The config root node.
 * @param entry Config map entry.
 * @param value Option argument value.
 * @param err   Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
static int sconf_opts_handle_option_str(struct SConfNode *root,
                                        const struct SConfMap *entry,
                                        const char *value, struct SConfErr *err)
{
    assert(root);
    assert(entry);
    assert(value);

    if (sconf_set_str(root, entry->path, value, err) == -1) {
        return -1;
    }

    return 0;
}

/**
 * @internal
 * @brief Handle integer option.
 *
 * @param root  The config root node.
 * @param entry Config map entry.
 * @param value Option argument value.
 * @param err   Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
static int sconf_opts_handle_option_int(struct SConfNode *root,
                                        const struct SConfMap *entry,
                                        const char *value, struct SConfErr *err)
{
    assert(root);
    assert(entry);
    assert(value);

    int64_t integer;

    int r = sconf_string_to_integer(value, &integer, err);
    if (r == -1) {
        return -1;
    }
    if (r == 0) {
        if (entry->opts_long) {
            sconf_err_set(err, "expected integer for option --%s/-%c",
                          entry->opts_long, entry->opts_short);
        }
        else {
            sconf_err_set(err, "expected integer for option -%c",
                          entry->opts_short);
        }
        return -1;
    }

    if (sconf_set_int(root, entry->path, integer, err) == -1) {
        return -1;
    }

    return 0;
}

/**
 * @internal
 * @brief Handle floating-point number option.
 *
 * @param root  The config root node.
 * @param entry Config map entry.
 * @param value Option argument value.
 * @param err   Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
static int sconf_opts_handle_option_float(struct SConfNode *root,
                                          const struct SConfMap *entry,
                                          const char *value,
                                          struct SConfErr *err)
{
    assert(root);
    assert(entry);
    assert(value);

    double fp;

    int r = sconf_string_to_float(value, &fp, err);
    if (r == -1) {
        return -1;
    }
    if (r == 0) {
        if (entry->opts_long) {
            sconf_err_set(err, "expected floating-point number for option "
                          "--%s/-%c", entry->opts_long, entry->opts_short);
        }
        else {
            sconf_err_set(err, "expected floating-point number for option "
                          "-%c", entry->opts_short);
        }
        return -1;
    }

    if (sconf_set_float(root, entry->path, fp, err) == -1) {
        return -1;
    }

    return 0;
}

/**
 * @internal
 * @brief Handle boolean option.
 *
 * @param root  The config root node.
 * @param entry Config map entry.
 * @param value Option argument value.
 * @param err   Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
static int sconf_opts_handle_option_bool(struct SConfNode *root,
                                         const struct SConfMap *entry,
                                         const char *value,
                                         struct SConfErr *err)
{
    assert(root);
    assert(entry);

    bool boolean = true;

    /* Argument for boolean type is optional */
    if (value) {
        int r = sconf_string_to_bool(value, &boolean);
        if (r == 0) {
            if (entry->opts_long) {
                sconf_err_set(err, "expected boolean for option --%s/-%c",
                              entry->opts_long, entry->opts_short);
            }
            else {
                sconf_err_set(err, "expected boolean for option -%c",
                              entry->opts_short);
            }
            return -1;
        }
    }

    if (sconf_set_bool(root, entry->path, boolean, err) == -1) {
        return -1;
    }

    return 0;
}

/**
 * @internal
 * @brief Handle YAML file option.
 *
 * @param root  The config root node.
 * @param entry Config map entry.
 * @param value Option argument value.
 * @param err   Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
static int sconf_opts_handle_option_yaml_file(struct SConfNode *root,
                                              const struct SConfMap *entry,
                                              const char *value,
                                              struct SConfErr *err)
{
    assert(root);
    assert(entry);
    assert(value);

    if (sconf_yaml_read(root, value, err) == -1) {
        return -1;
    }
    if (sconf_set_str(root, entry->path, value, err) == -1) {
        return -1;
    }

    return 0;
}

/**
 * @internal
 * @brief Calculate how much padding that is needed in options.
 *
 * @param map Config map.
 *
 * @return padding needed.
 */
static uint8_t sconf_opts_usage_string_calculate_padding(const struct SConfMap *map)
{
    assert (map);

    uint8_t padding = 0;

    for (const struct SConfMap *entry = map; entry->type; entry++)
    {
        if (!entry->opts_short) {
            continue;
        }

        size_t len = 0;

        if (entry->opts_long) {
            len += strlen(entry->opts_long);
        }

        if (entry->arg_type) {
            len += strlen(entry->arg_type);
        }
        else {
            len += strlen(sconf_type_to_arg_type_str(entry->type));
        }

        if (len > UINT8_MAX) {
            /* Something went really wrong, so it does not really matter
               how much padding we add. */
            return 4;
        }

        if (len > padding) {
            padding = len;
        }
    }

    return padding + 4;
}

/**
 * @internal
 * @brief Add header to usage string.
 *
 * @param string Pointer to usage string.
 * @param prog   Program name.
 * @param desc   Usage description.
 *
 * @return length on success, <0 on error.
 */
static int sconf_opts_usage_string_add_header(char *string, const char *prog,
                                              const char *desc)
{
    assert(prog);

    if (desc) {
        return snprintf(string, SCONF_OPTS_USAGE_STRING_MAX - 1,
                        "USAGE: %s\n%s\n\nOPTIONS:\n", prog, desc);
    }

    return snprintf(string, SCONF_OPTS_USAGE_STRING_MAX - 1,
                    "USAGE: %s\n\nOPTIONS:\n", prog);
}

/**
 * @internal
 * @brief Add option to usage string.
 *
 * @param string  Pointer to usage string.
 * @param size    Size of string.
 * @param curr    Config map entry.
 * @param padding Number of whitespace characters to pad line.
 *
 * @return size of string on success, -1 on error.
 */
static int sconf_opts_usage_string_add_option(char *string, int size,
                                              const struct SConfMap *curr,
                                              uint8_t padding)
{
    assert(string);
    assert(curr);
    assert(size > 0);

    /* Add short option */
    int len = snprintf(string + size, SCONF_OPTS_USAGE_STRING_MAX - 1 - size,
                       "\t-%c ", curr->opts_short);
    if (len < 0) {
        return -1;
    }

    size += len;

    /* Add long option */
    if (curr->opts_long) {
        len = snprintf(string + size, SCONF_OPTS_USAGE_STRING_MAX - 1 - size,
                       "--%s ", curr->opts_long);
        if (len < 0) {
            return -1;
        }

        size += len;
        padding -= len;
    }

    /* Add argument type */
    const char *arg_type;
    if (curr->arg_type) {
        arg_type = curr->arg_type;
    }
    else {
        arg_type = sconf_type_to_arg_type_str(curr->type);
    }

    len = snprintf(string + size, SCONF_OPTS_USAGE_STRING_MAX - 1 - size,
                   "%s", arg_type);
    if (len < 0) {
        return -1;
    }

    size += len;
    padding -= len;

    /* Add padding and help string */
    if (curr->help) {
        len = snprintf(string + size, SCONF_OPTS_USAGE_STRING_MAX - 1 - size,
                       "%*s: %s\n", padding, "", curr->help);
    }
    else {
        len = snprintf(string + size, SCONF_OPTS_USAGE_STRING_MAX - 1 - size,
                       "%*s:\n", padding, "");
    }

    if (len < 0) {
        return -1;
    }

    size += len;

    return size;
}

/**
 * @internal
 * @brief Generate usage string.
 *
 * @param prog   Program name.
 * @param string Pointer to usage string.
 * @param curr   Current config map entry.
 * @param map    Config map.
 * @param err    Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
static int sconf_opts_usage_string_generate(const char *prog,
                                            char *string,
                                            const struct SConfMap *curr,
                                            const struct SConfMap *map,
                                            struct SConfErr *err)
{
    assert(prog);
    assert(string);
    assert(curr);
    assert(map);

    int size = sconf_opts_usage_string_add_header(string, prog,
                                                  curr->usage_desc);
    if (size < 0) {
        sconf_err_set(err, "error generating usage string");
        return -1;
    }

    size_t padding = sconf_opts_usage_string_calculate_padding(map);

    for (const struct SConfMap *entry = map; entry->type; entry++)
    {
        if (!entry->opts_short) {
            continue;
        }

        size = sconf_opts_usage_string_add_option(string, size, entry, padding);
        if (size == -1) {
            sconf_err_set(err, "error generating usage string");
            return -1;
        }
    }

    return 0;
}

/**
 * @internal
 * @brief Handle usage option.
 *
 * @param root  The config root node.
 * @param entry Config map entry.
 * @param map   Config map.
 * @param used  User-supplied data passed to usage callback function.
 * @param prog  Program name.
 * @param err   Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
static int sconf_opts_handle_option_usage(struct SConfNode *root,
                                          const struct SConfMap *entry,
                                          const struct SConfMap *map,
                                          void *user, const char *prog,
                                          struct SConfErr *err)
{
    assert(root);
    assert(entry);
    assert(map);

    char usage[SCONF_OPTS_USAGE_STRING_MAX] = {0};

    int r = sconf_opts_usage_string_generate(prog, usage, entry, map, err);
    if (r == -1) {
        return -1;
    }

    if (entry->usage_func) {
        entry->usage_func(usage, user);
    }
    else {
        /* Default action if no usage callback function is specified */
        printf("%s\n", usage);
        sconf_node_destroy(root);
        exit(EXIT_SUCCESS);
    }

    return 0;
}

/**
 * @internal
 * @brief Handle option returned by `getopt_long`.
 *
 * @param root  The config root node.
 * @param entry Config map entry.
 * @param map   Config map.
 * @param value Option argument value.
 * @param user  User-supplied data passed to usage callback function.
 * @param prog  Program name.
 * @param err   Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
static int sconf_opts_handle_option(struct SConfNode *root,
                                    const struct SConfMap *entry,
                                    const struct SConfMap *map,
                                    const char *value, void *user,
                                    const char *prog, struct SConfErr *err)
{
    assert(root);
    assert(entry);

    switch (entry->type)
    {
        case SCONF_TYPE_STR:
            if (sconf_opts_handle_option_str(root, entry, value, err) == -1) {
                return -1;
            }
            break;

        case SCONF_TYPE_INT:
            if (sconf_opts_handle_option_int(root, entry, value, err) == -1) {
                return -1;
            }
            break;

        case SCONF_TYPE_FLOAT:
            if (sconf_opts_handle_option_float(root, entry, value, err) == -1) {
                return -1;
            }
            break;

        case SCONF_TYPE_BOOL:
            if (sconf_opts_handle_option_bool(root, entry, value, err) == -1) {
                return -1;
            }
            break;

        case SCONF_TYPE_YAML_FILE:
            if (sconf_opts_handle_option_yaml_file(root, entry,
                                                   value, err) == -1) {
                return -1;
            }
            break;

        case SCONF_TYPE_USAGE:
            if (sconf_opts_handle_option_usage(root, entry, map,
                                               user, prog, err) == -1) {
                return -1;
            }
            break;

        default:
            sconf_err_set(err, "type %s cannot be used for options",
                          sconf_type_to_str(entry->type));
            return -1;
    }

    return 0;
}

/**
 * @brief Parse application arguments based on config map.
 *
 * @param root The config root node.
 * @param map  Config map.
 * @param argc Number of arguments.
 * @param argv Array of arguments.
 * @param user User-supplied data passed to usage callback function.
 * @param err  Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
int sconf_opts_parse(struct SConfNode *root, const struct SConfMap *map,
                     int argc, char **argv, void *user, struct SConfErr *err)
{
    if (!root) {
        sconf_err_set(err, "no root specified when parsing opts");
        return -1;
    }

    if (!map) {
        sconf_err_set(err, "no map specified when parsing opts");
        return -1;
    }

    if (argc <= 1) {
        /* No command-line arguments specified */
        return 0;
    }

    const struct SConfMap *opts_index[SCONF_OPTS_INDEX_SIZE] = {0};

    int r = sconf_short_opts_index_create(map, opts_index, err);
    if (r == -1) {
        return -1;
    }

    struct option long_opts[SCONF_OPTS_LONG_OPTS_SIZE] = {0};

    r = sconf_opts_create_long_opts(map, long_opts, err);
    if (r == -1) {
        return -1;
    }

    char optstring[SCONF_OPTS_OPTSTRING_MAX_SIZE] = {0};

    r = sconf_opts_create_optstring(map, optstring, err);
    if (r == -1) {
        return -1;
    }

    if (strlen(optstring) == 1) {
        /* No command-line options configured */
        return 0;
    }

    /* Reset getopt state, to allow multiple runs, if necessary */
    optind = 0;

    /* Make getopt stop printing errors to stderr */
    opterr = 0;

    int c;
    while ((c = getopt_long(argc, argv, optstring, long_opts, NULL)) != -1)
    {
        if (c == ':') {
            sconf_err_set(err, "option '%c' requires an argument", optopt);
            return -1;
        }

        if (c == '?') {
            if (optopt == 0) {
                /* Unsupported long option */
                sconf_err_set(err, "unsupported option '%s' specified",
                              argv[optind - 1]);
            }
            else {
                /* Unsupported short option */
                sconf_err_set(err, "unsupported option '-%c' specified",
                              optopt);
            }
            return -1;
        }

        if (!opts_index[c]) {
            /* This should never happen */
            sconf_err_set(err, "option index for '%c' is NULL", c);
            return -1;
        }

        r = sconf_opts_handle_option(root, opts_index[c], map, optarg, user,
                                     argv[0], err);
        if (r == -1) {
            return -1;
        }
    }

    /* Check for non-option arguments */
    if (optind < argc) {
        sconf_err_set(err, "non-option arguments are unsupported, found '%s'",
                      argv[optind]);
        return -1;
    }

    return 0;
}

