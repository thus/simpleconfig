#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "art.h"
#include "sconf_private.h"

/* The delimiter used to split the path */
#define SCONF_PATH_DELIMITER "."

/**
 * Used to look up string representation of node types.
 */
static const char *sconf_types[] = {
    "unknown",
    "dictionary",
    "array",
    "string",
    "integer",
    "boolean",
    "floating-point number",
    "YAML file",
    "usage",
    "not-used"  /* used when >= SCONF_TYPE_MAX */
};

/**
 * Used to look up arg type string of node types.
 */
static const char *sconf_arg_types[] = {
    "TYPE NOT USED FOR OPTIONS",
    "TYPE NOT USED FOR OPTIONS",
    "TYPE NOT USED FOR OPTIONS",
    "<str>",
    "<int>",
    "",
    "<float>",
    "<file>",
    "",
    "TYPE NOT USED FOR OPTIONS"  /* used when >= SCONF_TYPE_MAX */
};

/**
 * @brief Return type string for specified node type.
 *
 * @param type Config node type.
 *
 * @return type string.
 */
const char *sconf_type_to_str(uint8_t type)
{
    if (type < SCONF_TYPE_MAX) {
        return sconf_types[type];
    }

    return sconf_types[SCONF_TYPE_MAX];
}

/**
 * @brief Return arg type string for specified node type.
 *
 * @param tupe Config node type.
 *
 * @return arg type string.
 */
const char *sconf_type_to_arg_type_str(uint8_t type)
{
    if (type < SCONF_TYPE_MAX) {
        return sconf_arg_types[type];
    }

    return sconf_arg_types[SCONF_TYPE_MAX];
}

/**
 * @brief Return string from node.
 *
 * @param node Config node.
 *
 * @return string from node.
 */
const char *sconf_str(const struct SConfNode *node)
{
    return node->string;
}

/**
 * @brief Return integer from node.
 *
 * @param node Config node.
 *
 * @return integer from node.
 */
int64_t sconf_int(const struct SConfNode *node)
{
    return node->integer;
}

/**
 * @brief Return floating-point number from node.
 *
 * @param node Config node.
 *
 * @return float from node.
 */
double sconf_float(const struct SConfNode *node)
{
    return node->fp;
}

/**
 * @brief Return boolean from node.
 *
 * @param node Config node.
 *
 * @return boolean from node.
 */
bool sconf_bool(const struct SConfNode *node)
{
    return node->boolean;
}

/**
 * @brief Return true if bool in node is true.
 *
 * @param node Config node.
 *
 * @return true if true.
 */
bool sconf_true(const struct SConfNode *node)
{
    return node->boolean ? true : false;
}

/**
 * @brief Return false if bool in node is true.
 *
 * @param node Config node.
 *
 * @return false if true.
 */
bool sconf_false(const struct SConfNode *node)
{
    return node->boolean ? false : true;
}

/**
 * @brief Return type from node.
 *
 * @param node Config node.
 *
 * @return type from node.
 */
uint8_t sconf_type(const struct SConfNode *node)
{
    return node->type;
}

/**
 * @brief Set error message.
 *
 * @param err Pointer to error struct.
 * @param fmt Format string.
 * @param ... Variable arguments.
 */
void sconf_err_set(struct SConfErr *err, const char *fmt, ...)
{
    assert(fmt);

    if (!err) {
        return;
    }

    /* Reset buffer each time it is used */
    memset(err->msg, 0, ERR_MSG_MAX_LEN);

    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(err->msg, ERR_MSG_MAX_LEN - 1, fmt, ap);
    va_end(ap);

    if (n > ERR_MSG_MAX_LEN - 1) {
        strcpy(err->msg, "setting error message failed");
    }
}

/**
 * @brief Get error message.
 *
 * @param err Pointer to error struct.
 *
 * @return error message.
 */
const char *sconf_strerror(struct SConfErr *err)
{
    return err->msg;
}

/**
 * @internal
 * @brief Get index of array from string.
 *
 * @param str   String to get index from.
 * @param index Pointer to index integer to set.
 * @param err   Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
static int sconf_array_get_index_from_string(const char *str,
                                             uint32_t *index,
                                             struct SConfErr *err)
{
    assert(str);

    if ((str[0] != '[') || (str[strlen(str) -1] != ']')) {
        sconf_err_set(err, "array index must be between brackets");
        return -1;
    }

    char *endptr;
    errno = 0;

    /* Skip start bracket */
    str++;

    long num = strtol(str, &endptr, 10);

    if (errno != 0) {
        sconf_err_set(err, "could not get array index '%s': %s", --str,
                      strerror(errno));
        return -1;
    }

    if (endptr == str) {
        sconf_err_set(err, "no digits found in array index '%s'", --str);
        return -1;
    }

    *index = num;

    return 0;
}

/**
 * @brief Insert config node in dictionary.
 *
 * @param name   The name of the node to insert.
 * @param parent Parent to add node to.
 * @param node   Config node to add.
 * @param err    Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
int sconf_node_dict_insert(const char *name, struct SConfNode *parent,
                           struct SConfNode *node, struct SConfErr *err)
{
    if (!name) {
        sconf_err_set(err, "name was not specified");
        return -1;
    }

    if (!parent) {
        sconf_err_set(err, "parent was not specified");
        return -1;
    }

    if (!node) {
        sconf_err_set(err, "node was not specified");
        return -1;
    }

    if (parent->type != SCONF_TYPE_DICT) {
        sconf_err_set(err, "parent node is not a dict");
        return -1;
    }

    if (art_insert(&parent->dictionary, (unsigned char *)name,
                   (int)strlen(name), node) != NULL) {
        sconf_err_set(err, "inserting node into dict failed");
        return -1;
    }

    return 0;
}

/**
 * @brief Search for node in dictionary.
 *
 * @param name   The name to search for.
 * @param parent Parent to search for node in.
 * @param node   Pointer to node, if found.
 * @param err    Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
int sconf_node_dict_search(const char *name, struct SConfNode *parent,
                           struct SConfNode **node, struct SConfErr *err)
{
    if (!name) {
        sconf_err_set(err, "name was not specified");
        return -1;
    }

    if (!parent) {
        sconf_err_set(err, "parent was not specified");
        return -1;
    }

    if (parent->type != SCONF_TYPE_DICT) {
        sconf_err_set(err, "parent node is not a dict");
        return -1;
    }

    *node = (struct SConfNode *)art_search(&parent->dictionary,
                                           (unsigned char *)name,
                                           (int)strlen(name));

    return 0;
}

/* Struct only used to pass needed pointers to art_iter callback when
   implementing dictionary iterator. */
struct SConfDictIterData {
    int (*cb)(const unsigned char *, struct SConfNode *, void *,
              struct SConfErr *);
    void *user;
    struct SConfErr *err;
};

/**
 * @internal
 * @brief art_iter callback function used when iterating over dictionary.
 *
 * @param data    User-supplied data.
 * @param key     Key from dictionary.
 * @param key_len Length of the key.
 * @param value   Value from dictionary.
 *
 * @return 0 on success, -1 otherwise.
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
static int sconf_node_dict_foreach_iter_cb(void *data, const unsigned char *key,
                                           uint32_t key_len, void *value)
{
    assert(data);
    assert(key);
    assert(value);

    struct SConfDictIterData *iter_data = (struct SConfDictIterData *)data;
    struct SConfNode *node = (struct SConfNode *)value;

    if (iter_data->cb(key, node, iter_data->user, iter_data->err) != 0) {
        return -1;
    }

    return 0;
}
#pragma GCC diagnostic pop

/**
 * @brief Iterate over all nodes in dictionary.
 *
 * @param dict Dictionary to iterate over.
 * @param cb   Callback function.
 * @param user User-supplied data passed to callback function.
 * @param err  Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
int sconf_node_dict_foreach(struct SConfNode *dict,
                            int (*cb)(const unsigned char *name,
                                      struct SConfNode *node,
                                      void *user, struct SConfErr *err),
                            void *user, struct SConfErr *err)
{
    if (!dict) {
        sconf_err_set(err, "dictionary is not specified");
        return -1;
    }

    if (!cb) {
        sconf_err_set(err, "callback function must be specified");
        return -1;
    }

    if (dict->type != SCONF_TYPE_DICT) {
        sconf_err_set(err, "could not use dictionary iterator on node type %s",
                      sconf_type_to_str(dict->type));
        return -1;
    }

    struct SConfDictIterData iter_data = { cb, user, err };
    int r = art_iter(&dict->dictionary, sconf_node_dict_foreach_iter_cb,
                     &iter_data);
    if (r != 0) {
        return -1;
    }

    return 0;
}

/**
 * @brief Insert config node in array.
 *
 * @param index  Where in array to insert.
 * @param parent Parent to add node to.
 * @param node   Config node to add.
 * @param err    Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
int sconf_node_array_insert(uint32_t index, struct SConfNode *parent,
                            struct SConfNode *node, struct SConfErr *err)
{
    if (!parent) {
        sconf_err_set(err, "parent was not specified");
        return -1;
    }

    if (!node) {
        sconf_err_set(err, "node was not specified");
        return -1;
    }

    if (parent->type != SCONF_TYPE_ARRAY) {
        sconf_err_set(err, "parent node is not an array");
        return -1;
    }

    if (sconf_array_insert(parent->array, index, node, err) == -1) {
        return -1;
    }

    return 0;
}

/**
 * @brief Search for node in array.
 *
 * @param index  Where in the array to look.
 * @param parent Parent to search for node in.
 * @param node   Pointer to node, if found.
 * @param err    Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
int sconf_node_array_search(uint32_t index, struct SConfNode *parent,
                            struct SConfNode **node, struct SConfErr *err)
{
   if (!parent) {
        sconf_err_set(err, "parent was not specified");
        return -1;
    }

    if (parent->type != SCONF_TYPE_ARRAY) {
        sconf_err_set(err, "parent node is not a dict");
        return -1;
    }

    if (!parent->array || !parent->array->entries) {
        sconf_err_set(err, "parent array is not initialized");
        return -1;
    }

    if (parent->array->size > index) {
        *node = parent->array->entries[index];
    }

    return 0;
}

/**
 * @brief Iterate over nodes in array.
 *
 * @param array Array to iterate over.
 * @param node  Pointer to node to set.
 * @param next  Pointer to the position in the array.
 * @param err   Pointer to error struct.
 *
 * @return 1 if a node is set, 0 if the end is reached, or -1 on error.
 */
int sconf_node_array_next(struct SConfNode *array, struct SConfNode **node,
                          uint32_t *next, struct SConfErr *err)
{
    if (!array) {
        sconf_err_set(err, "array is not specified");
        return -1;
    }

    if (array->type != SCONF_TYPE_ARRAY) {
        sconf_err_set(err, "could not use array iterator on node type %s",
                      sconf_type_to_str(array->type));
        return -1;
    }

    for (uint32_t i = *next; i < array->array->size; i++)
    {
        if (array->array->entries[i] == NULL) {
            continue;
        }

        *next = i + 1;
        *node = array->array->entries[i];
	return 1;
    }

    return 0;
}
		                        
/**
 * @brief Iterate over nodes in array using callback function.
 *
 * @param array Array to iterate over.
 * @param cb    Callback function.
 * @param user  User-supplied data passed to callback function.
 * @param err   Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
int sconf_node_array_foreach(struct SConfNode *array,
                              int (*cb)(uint32_t index, struct SConfNode *node,
                                        void *user, struct SConfErr *err),
                              void *user, struct SConfErr *err)
{
    if (!array) {
        sconf_err_set(err, "array is not specified");
        return -1;
    }

    if (!cb) {
        sconf_err_set(err, "callback function must be specified");
        return -1;
    }

    if (array->type != SCONF_TYPE_ARRAY) {
        sconf_err_set(err, "could not use array iterator on node type %s",
                      sconf_type_to_str(array->type));
        return -1;
    }

    for (uint32_t i = 0; i < array->array->size; i++)
    {
        if (!array->array->entries[i]) {
            continue;
        }

        if (cb(i, array->array->entries[i], user, err) != 0) {
            return -1;
        }
    }

    return 0;
}

/**
 * @internal
 * @brief Callback function to iterate used when destroying dictionary.
 *
 * @param data    User-supplied data.
 * @param key     Key from dictionary.
 * @param key_len Length of the key.
 * @param value   Value from dictionary.
 *
 * @return 0 on success.
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
static int sconf_node_dict_destroy_iter_cb(void *data, const unsigned char *key,
                                           uint32_t key_len, void *value)
{
    assert(key);
    assert(value);

    struct SConfNode *node = (struct SConfNode *)value;

    sconf_node_destroy(node);

    return 0;
}
#pragma GCC diagnostic pop

/**
 * @internal
 * @brief Destroy config node dictionary.
 *
 * @param node The config node.
 */
static void sconf_node_dict_destroy(struct SConfNode *node)
{
    assert(node);
    assert(node->type == SCONF_TYPE_DICT);

    art_iter(&node->dictionary, sconf_node_dict_destroy_iter_cb, NULL);
    art_tree_destroy(&node->dictionary);
}

/**
 * @internal
 * @brief Destroy config node array.
 *
 * @param node The config node.
 */
static void sconf_node_array_destroy(struct SConfNode *node)
{
    assert(node);
    assert(node->type == SCONF_TYPE_ARRAY);
    assert(node->array);
    assert(node->array->entries);

    for (uint32_t i = 0; i < node->array->size; i++)
    {
        if (!node->array->entries[i]) {
            continue;
        }

        sconf_node_destroy(node->array->entries[i]);
    }

    sconf_array_destroy(node->array);
}

/**
 * @internal
 * @brief Destroy config node string.
 *
 * @param node The config node.
 */
static void sconf_node_str_destroy(struct SConfNode *node)
{
    assert(node);
    assert(node->type == SCONF_TYPE_STR);

    if (node->string) {
        free(node->string);
        node->string = NULL;
    }
}

/**
 * @brief Destroy a node.
 *
 * @param node The config node.
 */
void sconf_node_destroy(struct SConfNode *node)
{
    if (!node) {
        return;
    }

    switch (node->type)
    {
       case SCONF_TYPE_DICT:
            sconf_node_dict_destroy(node);
            break;
        case SCONF_TYPE_ARRAY:
            sconf_node_array_destroy(node);
            break;
        case SCONF_TYPE_STR:
            sconf_node_str_destroy(node);
            break;
    }

    free(node);
}

/**
 * @internal
 * @brief Initialize dictionary config node.
 *
 * @param node The config node.
 * @param err  Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
static int sconf_node_dict_init(struct SConfNode *node, struct SConfErr *err)
{
    assert(node);
    assert(node->type == SCONF_TYPE_DICT);

    int r = art_tree_init(&node->dictionary);
    if (r != 0) {
        sconf_err_set(err, "failed to create dict node tree");
        return -1;
    }

    return 0;
}

/**
 * @internal
 * @brief Initialize array config node.
 *
 * @param node The config node.
 * @param err  Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
static int sconf_node_array_init(struct SConfNode *node, struct SConfErr *err)
{
    assert(node);
    assert(node->type == SCONF_TYPE_ARRAY);

    node->array = sconf_array_create(1, err);
    if (!node->array) {
        return -1;
    }

    return 0;
}

/**
 * @internal
 * @brief Initialize string config node.
 *
 * @param node The config node.
 * @param data Data used when initializing node.
 * @param err  Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
static int sconf_node_str_init(struct SConfNode *node, void *data,
                               struct SConfErr *err)
{
    assert(node);
    assert(node->type == SCONF_TYPE_STR);
    assert(data);

    char *str = (char *)data;

    /* Allow overwrite */
    if (node->string) {
        free(node->string);
        node->string = NULL;
    }

    node->string = strdup(str);
    if (!node->string) {
        sconf_err_set(err, "failed to allocate memory for node string");
        return -1;
    }

    return 0;
}

/**
 * @internal
 * @brief Initialize integer config node.
 *
 * @param node The config node.
 * @param data Data used when initializing node.
 * @param err  Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
static int sconf_node_int_init(struct SConfNode *node, void *data)
{
    assert(node);
    assert(node->type == SCONF_TYPE_INT);
    assert(data);

    int64_t *integer = (int64_t *)data;

    node->integer = *integer;

    return 0;
}

/**
 * @internal
 * @brief Initialize boolean config node.
 *
 * @param node The config node.
 * @param data Data used when initializing node.
 * @param err  Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
static int sconf_node_bool_init(struct SConfNode *node, void *data)
{
    assert(node);
    assert(node->type == SCONF_TYPE_BOOL);
    assert(data);

    bool *boolean = (bool *)data;

    node->boolean = *boolean;

    return 0;
}

/**
 * @internal
 * @brief Initialize floating point number config node.
 *
 * @param node The config node.
 * @param data Data used when initializing node.
 * @param err  Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
static int sconf_node_float_init(struct SConfNode *node, void *data)
{
    assert(node);
    assert(node->type == SCONF_TYPE_FLOAT);
    assert(data);

    double *fp = (double *)data;

    node->fp = *fp;

    return 0;
}

/**
 * @brief Create a new config node.
 *
 * @param type The type of node to create.
 * @param data Data used when creating node.
 * @param err  Pointer to error struct.
 *
 * @return Created node on success, NULL otherwise.
 */
struct SConfNode *sconf_node_create(int type, void *data, struct SConfErr *err)
{
    int r = 0;

    struct SConfNode *node = calloc(1, sizeof(struct SConfNode));
    if (!node) {
        sconf_err_set(err, "could not allocate memory for node");
        return NULL;
    }

    node->type = type;

    switch (type)
    {
        case SCONF_TYPE_DICT:
            r = sconf_node_dict_init(node, err);
            break;
        case SCONF_TYPE_ARRAY:
            r = sconf_node_array_init(node, err);
            break;
        case SCONF_TYPE_STR:
            r = sconf_node_str_init(node, data, err);
            break;
        case SCONF_TYPE_INT:
            r = sconf_node_int_init(node, data);
            break;
        case SCONF_TYPE_BOOL:
            r = sconf_node_bool_init(node, data);
            break;
        case SCONF_TYPE_FLOAT:
            r = sconf_node_float_init(node, data);
            break;
        default:
            sconf_err_set(err, "create: unknown node type '%d'", type);
            sconf_node_destroy(node);
            return NULL;
    }

    if (r == -1) {
        free(node);
        return NULL;
    }

    return node;
}

/**
 * @brief Create config node if it does not exist.
 *
 * @param name   The name of the config node.
 * @param type   The type of the config node.
 * @param parent Pointer to the parent.
 * @param index  Index to use if parent is an array.
 * @param data   Data used when creating node.
 * @param err    Pointer to the error struct.
 *
 * @return Pointer to node created on success, NULL otherwise.
 */
struct SConfNode *sconf_node_create_and_insert(const char *name, uint8_t type,
                                               struct SConfNode *parent,
                                               uint32_t index, void *data,
                                               struct SConfErr *err)
{
    assert(parent);

    int r = 0;
    struct SConfNode *node = NULL;

    if (name && name[0] == '[' && parent->type == SCONF_TYPE_ARRAY) {
        r = sconf_array_get_index_from_string(name, &index, err);
        if (r == -1) {
            return NULL;
        }
    }

    switch (parent->type)
    {
        case SCONF_TYPE_DICT:
            r = sconf_node_dict_search(name, parent, &node, err);
            break;
        case SCONF_TYPE_ARRAY:
            r = sconf_node_array_search(index, parent, &node, err);
            break;
        default:
            sconf_err_set(err, "parent node must be dict or array");
            return NULL;
    }

    if (r == -1) {
        return NULL;
    }

    if (node) {
        if (node->type != type) {
            sconf_err_set(err, "node '%s' already exist, but types does not "
                          "match ('%s' != '%s')", name, sconf_type_to_str(type),
                          sconf_type_to_str(node->type));
            return NULL;
        }

        /* Replace value of node */
        switch (type)
        {
            case SCONF_TYPE_STR:
                r = sconf_node_str_init(node, data, err);
                break;
            case SCONF_TYPE_INT:
                r = sconf_node_int_init(node, data);
                break;
            case SCONF_TYPE_BOOL:
                r = sconf_node_bool_init(node, data);
                break;
            case SCONF_TYPE_FLOAT:
                r = sconf_node_float_init(node, data);
                break;
        }

        if (r == -1) {
            return NULL;
        }

        return node;
    }

    /* Node does not exist, so create it */
    node = sconf_node_create(type, data, err);
    if (!node) {
        return NULL;
    }

    switch (parent->type)
    {
        case SCONF_TYPE_DICT:
            r = sconf_node_dict_insert(name, parent, node, err);
            break;
        case SCONF_TYPE_ARRAY:
            r = sconf_node_array_insert(index, parent, node, err);
            break;
    }

    if (r == -1) {
        sconf_node_destroy(node);
        return NULL;
    }

    return node;
}

/**
 * @brief Get config node based on path.
 *
 * @param root Pointer to root config node.
 * @param path The path to the config node to get.
 * @param node Pointer to node, if found.
 * @param err  Pointer to error struct.
 *
 * @return 1 on found, 0 on not found, -1 on error.
 */
int sconf_get(struct SConfNode *root, const char *path, struct SConfNode **node,
              struct SConfErr *err)
{
    if (!root) {
        sconf_err_set(err, "no root was specified");
        return -1;
    }

    if (!path) {
        sconf_err_set(err, "no path was provided");
        return -1;
    }

    char *copy = strdup(path);
    if (!copy) {
        sconf_err_set(err, "failed to copy path string");
        return -1;
    }

    struct SConfNode *parent = root;
    uint32_t parent_index = 0;

    int r = 0;
    char *save_ptr = NULL;
    char *next = strtok_r(copy, SCONF_PATH_DELIMITER, &save_ptr);
    char *curr = NULL;

    while (next != NULL)
    {
       if (curr) {
            if (curr[0] == '[' && parent->type != SCONF_TYPE_ARRAY) {
                /* Not found */
                free(copy);
                return 0;
            }

            struct SConfNode *tmp = NULL;

            switch (parent->type)
            {
                case SCONF_TYPE_DICT:
                    r = sconf_node_dict_search(curr, parent, &tmp, err);
                    break;
                case SCONF_TYPE_ARRAY:
                    r = sconf_node_array_search(parent_index, parent, &tmp,
                                                err);
                    break;
                default:
                    sconf_err_set(err, "parent node must be dict or array");
                    free(copy);
                    return -1;
            }

            if (r == -1) {
                free(copy);
                return -1;
            }

            parent = tmp;

            if (parent == NULL) {
                /* Not found */
                free(copy);
                return 0;
            }
        }

        if (parent->type == SCONF_TYPE_ARRAY && next[0] == '[') {
            /* Used for the next node when inserting into the array */
            r = sconf_array_get_index_from_string(next, &parent_index, err);
            if (r == -1) {
                free(copy);
                return -1;
            }
        }

        curr = next;
        next = strtok_r(NULL, SCONF_PATH_DELIMITER, &save_ptr);
    }

    struct SConfNode *found = NULL;

    switch (parent->type)
    {
        case SCONF_TYPE_DICT:
            r = sconf_node_dict_search(curr, parent, &found, err);
            break;
        case SCONF_TYPE_ARRAY:
            r = sconf_node_array_search(parent_index, parent, &found, err);
            break;
        default:
            sconf_err_set(err, "parent node must be dict or array");
            free(copy);
            return -1;
    }

    free(copy);

    if (r == -1) {
        return -1;
    }

    if (found) {
        *node = found;
        return 1;
    }

    return 0;
}

/**
 * @brief Get config string based on path.
 *
 * @param root Pointer to root config node.
 * @param path The path to the config node to get.
 * @param str  Pointer to string, if found.
 * @param err  Pointer to error struct.
 *
 * @return 1 on found, 0 on not found, -1 on error.
 */
int sconf_get_str(struct SConfNode *root, const char *path, const char **str,
                  struct SConfErr *err)
{
    struct SConfNode *node = NULL;
    int r = sconf_get(root, path, &node, err);
    if (r != 1) {
        return r;
    }

    if (node == NULL) {
        sconf_err_set(err, "node found but node is NULL");
        return -1;
    }

    if (node->type != SCONF_TYPE_STR) {
        sconf_err_set(err, "config node '%s' is %s not %s",
                      path, sconf_type_to_str(node->type),
                      sconf_type_to_str(SCONF_TYPE_STR));
        return -1;
    }

    *str = node->string;

    return 1;
}

/**
 * @brief Get config integer based on path.
 *
 * @param root    Pointer to root config node.
 * @param path    The path to the config node to get.
 * @param integer Pointer to integer, if found.
 * @param err     Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
int sconf_get_int(struct SConfNode *root, const char *path,
                  const int64_t **integer, struct SConfErr *err)
{
    struct SConfNode *node = NULL;

    int r = sconf_get(root, path, &node, err);
    if (r != 1) {
        return r;
    }

    if (node == NULL) {
        sconf_err_set(err, "node found but node is NULL");
        return -1;
    }

    if (node->type != SCONF_TYPE_INT) {
        sconf_err_set(err, "config node '%s' is %s not %s",
                      path, sconf_type_to_str(node->type),
                      sconf_type_to_str(SCONF_TYPE_INT));
        return -1;
    }

    *integer = &node->integer;

    return 1;
}

/**
 * @brief Get config boolean based on path.
 *
 * @param root    Pointer to root config node.
 * @param path    The path to the config node to get.
 * @param boolean Pointer to boolean, if found.
 * @param err     Pointer to error struct.
 *
 * @return 1 on found, 0 on not found, -1 on error.
 */
int sconf_get_bool(struct SConfNode *root, const char *path,
                   const bool **boolean, struct SConfErr *err)
{
    struct SConfNode *node = NULL;

    int r = sconf_get(root, path, &node, err);
    if (r != 1) {
        return r;
    }

    if (node == NULL) {
        sconf_err_set(err, "node found but node is NULL");
        return -1;
    }

    if (node->type != SCONF_TYPE_BOOL) {
        sconf_err_set(err, "config node '%s' is %s not %s",
                      path, sconf_type_to_str(node->type),
                      sconf_type_to_str(SCONF_TYPE_BOOL));
        return -1;
    }

    *boolean = &node->boolean;

    return 1;
}

/**
 * @brief Get config floating point number based on path.
 *
 * @param root Pointer to root config node.
 * @param path The path to the config node to get.
 * @param fp   Pointer to floating point number, if found.
 * @param err  Pointer to error struct.
 *
 * @return 1 on found, 0 on not found, -1 on error.
 */
int sconf_get_float(struct SConfNode *root, const char *path,
                    const double **fp, struct SConfErr *err)
{
    struct SConfNode *node = NULL;

    int r = sconf_get(root, path, &node, err);
    if (r != 1) {
        return r;
    }

    if (node == NULL) {
        sconf_err_set(err, "node found but node is NULL");
        return -1;
    }

    if (node->type != SCONF_TYPE_FLOAT) {
        sconf_err_set(err, "config node '%s' is %s not %s",
                      path, sconf_type_to_str(node->type),
                      sconf_type_to_str(SCONF_TYPE_FLOAT));
        return -1;
    }

    *fp = &node->fp;

    return 1;
}

/**
 * @brief Set config value based on path.
 *
 * @param root  Pointer to root config node.
 * @param path  The path to the config node to set.
 * @param type  The type of node to set.
 * @param value The value to set the config node to.
 * @param err   Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
int sconf_set(struct SConfNode *root, const char *path, uint8_t type,
              void *value, struct SConfErr *err)
{
    if (!root) {
        sconf_err_set(err, "no root was specified");
        return -1;
    }

    if (!value) {
        sconf_err_set(err, "no value was specified");
        return -1;
    }

    if (!path) {
        sconf_err_set(err, "no path was provided");
        return -1;
    }

    char *copy = strdup(path);
    if (!copy) {
        sconf_err_set(err, "failed to copy path string");
        return -1;
    }

    struct SConfNode *parent = root;
    int parent_index = 0;

    int depth = 1;
    char *save_ptr = NULL;
    char *next = strtok_r(copy, SCONF_PATH_DELIMITER, &save_ptr);
    char *curr = NULL;

    /* Create all the parent nodes in the path */
    while (next != NULL)
    {
        if (curr) {
            uint8_t curr_type = SCONF_TYPE_DICT; 

            if (next[0] == '[') {
                curr_type = SCONF_TYPE_ARRAY;
            }
            
            parent = sconf_node_create_and_insert(curr, curr_type, parent,
                                                  parent_index, NULL, err);
            if (parent == NULL) {
                free(copy);
                return -1;
            }
        }

        if (depth >= SCONF_MAX_DEPTH) {
            sconf_err_set(err, "maximum depth reached when adding '%s'", path);
	    free(copy);
            return -1;
        }
        depth++;

        curr = next;
        next = strtok_r(NULL, SCONF_PATH_DELIMITER, &save_ptr);
    }

    struct SConfNode *node = sconf_node_create_and_insert(curr, type, parent,
                                                          parent_index, value,
                                                          err);
    if (!node) {
        free(copy);
        return -1;
    }

    free(copy);

    return 0;
}

/**
 * @brief Set config string based on path.
 *
 * @param root Pointer to root config node.
 * @param path The path to the config node to set.
 * @param str  The string to set.
 * @param err  Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
int sconf_set_str(struct SConfNode *root, const char *path, const char *str,
                  struct SConfErr *err)
{
   return sconf_set(root, path, SCONF_TYPE_STR, (void *)str, err);
}

/**
 * @brief Set config integer based on path.
 *
 * @param root    Pointer to root config node.
 * @param path    The path to the config node to set.
 * @param integer The integer to set.
 * @param err     Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
int sconf_set_int(struct SConfNode *root, const char *path, int64_t integer,
                  struct SConfErr *err)
{
   return sconf_set(root, path, SCONF_TYPE_INT, &integer, err);
}

/**
 * @brief Set config boolean based on path.
 *
 * @param root    Pointer to root config node.
 * @param path    The path to the config node to set.
 * @param boolean The boolean value to set.
 * @param err     Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
int sconf_set_bool(struct SConfNode *root, const char *path, bool boolean,
                   struct SConfErr *err)
{
   return sconf_set(root, path, SCONF_TYPE_BOOL, &boolean, err);
}

/**
 * @brief Set floating point number based on path.
 *
 * @param root Pointer to root config node.
 * @param path The path to the config node to set.
 * @param fp   The floating point number to set.
 * @param err  Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
int sconf_set_float(struct SConfNode *root, const char *path, double fp,
                    struct SConfErr *err)
{
   return sconf_set(root, path, SCONF_TYPE_FLOAT, &fp, err);
}

/**
 * @brief Create a complete config tree based on config map.
 *
 * @param root Pointer to root config node.
 * @param map  Config map.
 * @param argc Number of arguments.
 * @param argv Array of arguments.
 * @param user User-supplied data passed to callback functions.
 * @param err  Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
int sconf_initialize(struct SConfNode *root, const struct SConfMap *map,
                     int argc, char **argv, void *user, struct SConfErr *err)
{
    if (!root) {
        sconf_err_set(err, "root config node is NULL");
        return -1;
    }

    if (!map) {
        sconf_err_set(err, "config map is missing");
        return -1;
    }

    int r = sconf_opts_parse(root, map, argc, argv, user, err);
    if (r == -1) {
        return -1;
    }

    r = sconf_env_read(root, map, err);
    if (r == -1) {
        return -1;
    }

    r = sconf_defaults(root, map, err);
    if (r == -1) {
        return -1;
    }

    r = sconf_validate(root, map, user, err);
    if (r == -1) {
        return -1;
    }

    return 0;
}

