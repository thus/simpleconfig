#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

/* Maximum depth of a config node, e.g foo.bar.meh.lol has a depth of 4 */
#define SCONF_MAX_DEPTH 20

/* Maximum length of error messages */
#define ERR_MSG_MAX_LEN 256

struct SConfErr {
    char msg[ERR_MSG_MAX_LEN];
};

enum {
    SCONF_TYPE_UNKNOWN = 0,

    /* Node types */
    SCONF_TYPE_DICT,
    SCONF_TYPE_ARRAY,
    SCONF_TYPE_STR,
    SCONF_TYPE_INT,
    SCONF_TYPE_BOOL,
    SCONF_TYPE_FLOAT,

    /* Special types */
    SCONF_TYPE_YAML_FILE, /* used to automatically read YAML files */
    SCONF_TYPE_USAGE,     /* used in command-line options parsing */

    /* The ordering of types is used in sconf, so add new types here */

    SCONF_TYPE_MAX,
};

/**
 * Opaque pointer type to represent a config node (see src/sconf_private.h
 * for the private struct).
 */
struct SConfNode;

/**
 * Node access helper functions.
 *
 * These functions could be used to access the members of the SConfNode
 * struct. Although, it's more likely that you should use one of the type
 * specific sconf_get_* functions instead, for getting the value of the
 * config node. The only exception is sconf_type, which is used quite
 * often.
 *
 * Example:
 *   struct SConfNode *node;
 *   int r = sconf_get(root, "logging.log-dir", &node, &err);
 *   if (r == -1) {
 *       printf("Error: %s\n", sconf_strerror(&err));
 *       return EXIT_FAILURE;
 *   }
 *   else if (r == 1) {
 *       printf("log-dir: %s\n", sconf_str(node));
 *   }
 *
 *   r = sconf_get(root, "logging.enabled", &node, &err);
 *   if (r == -1) {
 *       printf("Error: %s\n", sconf_strerror(&err));
 *       return EXIT_FAILURE;
 *   }
 *   else if (r == 1) {
 *       if (sconf_true(node)) {
 *           printf("Logging is enabled!\n");
 *      }
 *       else {
 *           printf("No logging for you!\n");
 *       }
 *   }
 */
const char *sconf_str(const struct SConfNode *node);
int64_t sconf_int(const struct SConfNode *node);
double sconf_float(const struct SConfNode *node);
bool sconf_bool(const struct SConfNode *node);
bool sconf_true(const struct SConfNode *node);
bool sconf_false(const struct SConfNode *node);
uint8_t sconf_type(const struct SConfNode *node);

/**
 * Structure representing a config map.
 */
struct SConfMap {
    /* Path to node (e.g "foo.bar") */
    char *path;

    /* Node type (e.g SCONF_TYPE_STR) */
    uint8_t type;

    /* Long option used for command-line options (e.g "log-dir") */
    char *opts_long;

    /* Short option used for command-line options (e.g 'l') */
    char opts_short;

    /* Help string used when generating usage string (e.g "log directory") */
    char *help;

    /* Argument type used when generating usage string (e.g "<dir>") */
    char *arg_type;

    /* Description used when generating usage string */
    char *usage_desc;

    /* Environment variable (e.g "LOG_DIR") */
    char *env;

    /* Default value to use, applied after all other config */
    char *default_value;

    /* Specify if the config path is required (true/false) */
    bool required;

    /* Callback function used when generating usage string */
    void (*usage_func)(const char *, void *);

    /* Callback function used to validate config path */
    int (*validate_func)(const char *, const struct SConfNode *, void *,
                         struct SConfErr *);
};

/**
 * Create a new config node.
 *
 * Example:
 *   struct SConfNode *node = sconf_node_create(SCONF_TYPE_INT, 42, &err);
 *   if (!node) {
 *       printf("Error: %s\n", sconf_strerror(&err));
 *       return EXIT_FAILURE;
 *   }
 */
struct SConfNode *sconf_node_create(int type, void *data,
                                    struct SConfErr *err);

/**
 * Destroy an existing config node.
 *
 * Recursively destroys all children of the node.
 *
 * Example:
 *   sconf_node_destroy(node);
 */
void sconf_node_destroy(struct SConfNode *node);

/**
 * Macro for creating root config node.
 *
 * Example:
 *   struct SConfNode *root = SCONF_ROOT(&err);
 *   if (!root) {
 *       printf("Error: %s\n", sconf_strerror(&err));
 *       return EXIT_FAILURE;
 *   }
 */
#define SCONF_ROOT(err) sconf_node_create(SCONF_TYPE_DICT, NULL, err)

/**
 * Get config node at path.
 *
 * Unless getting a dictionary or array, you should probably use one of
 * the type specific sconf_get_* functions below instead.
 *
 * Example:
 *   struct SconfNode *node;
 *   int r = sconf_get(root, "foo.bar", &node, &err);
 *   if (r == -1) {
 *       printf("Error: %s\n", sconf_strerror(&err));
 *       return EXIT_FAILURE;
 *   }
 *   else if (r == 1) {
 *       printf("Found node of type %s\n", sconf_type_to_str(node->type));
 *   }
 */
int sconf_get(struct SConfNode *root, const char *path, struct SConfNode **node,
              struct SConfErr *err);

/**
 * Get string from config node at path.
 *
 * Example:
 *   const char *string;
 *   int r = sconf_get_str(root, "a.b.c", &string, &err);
 *   if (r == -1) {
 *       printf("Error: %s\n", sconf_strerror(&err));
 *       return EXIT_FAILURE;
 *   }
 *   else if (r == 1) {
 *       printf("%s\n", string);
 *   }
 */
int sconf_get_str(struct SConfNode *root, const char *path, const char **str,
                  struct SConfErr *err);

/**
 * Get integer from config node at path.
 *
 * Example:
 *   const int64_t *integer;
 *   int r = sconf_get_int(root, "number.one", &integer, &err);
 *   if (r == -1) {
 *       printf("Error: %s\n", sconf_strerror(&err));
 *       return EXIT_FAILURE;
 *   }
 *   else if (r == 1) {
 *       printf("%" PRId64 "\n", *integer);
 *   }
 */
int sconf_get_int(struct SConfNode *root, const char *path,
                  const int64_t **integer, struct SConfErr *err);

/**
 * Get boolean from config node at path.
 *
 * Example:
 *   const bool *boolean;
 *   int r = sconf_get_bool(root, "haz_cheeseburger", &boolean, &err);
 *   if (r == -1) {
 *       printf("Error: %s\n", sconf_strerror(&err));
 *       return EXIT_FAILURE;
 *   }
 *   else if (r == 1) {
 *       if (*boolean) {
 *           printf("Yes!\n");
 *       }
 *   }
 */
int sconf_get_bool(struct SConfNode *root, const char *path,
                   const bool **boolean, struct SConfErr *err);

/**
 * Get floating-point number from config node at path.
 *
 * Example:
 *   const double *fp;
 *   int r = sconf_get_float(root, "math.pi", &fp, &err);
 *   if (r == -1) {
 *       printf("Error: %s\n", sconf_strerror(&err));
 *       return EXIT_FAILURE;
 *   }
 *   else if (r == 1) {
 *       printf("pi is %f\n", *fp);
 *   }
 */
int sconf_get_float(struct SConfNode *root, const char *path, const double **fp,
		    struct SConfErr *err);

/**
 * Set config node of type to value at path.
 *
 * You should probably use one of the type specific sconf_set_* functions
 * below instead.
 *
 * Example:
 *   char *string = "foobar";
 *   int r = sconf_set(root, "foo", SCONF_TYPE_STR, string, &err);
 *   if (r == -1) {
 *       printf("Error: %s\n", sconf_strerror(&err));
 *       return EXIT_FAILURE;
 *   }
 */
int sconf_set(struct SConfNode *root, const char *path, uint8_t type,
              void *value, struct SConfErr *err);

/**
 * Set string in config node at path.
 *
 * Example:
 *   const char *string = "barfoo";
 *   int r = sconf_set_str(root, "bar", string, &err);
 *   if (r == -1) {
 *       printf("Error: %s\n", sconf_strerror(&err));
 *       return EXIT_FAILURE;
 *   }
 */
int sconf_set_str(struct SConfNode *root, const char *path, const char *str,
                  struct SConfErr *err);

/**
 * Set integer in config node at path.
 *
 * Example:
 *   int64_t integer = 123456789;
 *   int r = sconf_set_int(root, "num", integer, &err);
 *   if (r == -1) {
 *       printf("Error: %s\n", sconf_strerror(&err));
 *       return EXIT_FAILURE;
 *   }
 */
int sconf_set_int(struct SConfNode *root, const char *path, int64_t integer,
                  struct SConfErr *err);

/**
 * Set boolean in config node at path.
 *
 * Example:
 *   bool enable = true;
 *   int r = sconf_set_bool(root, "logging.enable", enable, &err);
 *   if (r == -1) {
 *       printf("Error: %s\n", sconf_strerror(&err));
 *       return EXIT_FAILURE;
 *   }
 */
int sconf_set_bool(struct SConfNode *root, const char *path, bool boolean,
                   struct SConfErr *err);

/**
 * Set floating-point number in config node at path.
 *
 * Example:
 *   double woop = 1.23;
 *   int r = sconf_set_float(root, "wooper", woop, &err);
 *   if (r == -1) {
 *       printf("Error: %s\n", sconf_strerror(&err));
 *       return EXIT_FAILURE;
 *   }
 */
int sconf_set_float(struct SConfNode *root, const char *path, double fp,
		    struct SConfErr *err);

/**
 * Insert node into dictionary.
 *
 * Example:
 *   struct SConfNode *node = sconf_node_create(SCONF_TYPE_STR, "foobar", &err);
 *   if (!node) {
 *       printf("Error: %s\n", sconf_strerror(&err));
 *       return EXIT_FAILURE;
 *   }
 *
 *   int r = sconf_node_dict_insert("foo", dict, node, &err);
 *   if (r == -1) {
 *       printf("Error: %s\n", sconf_strerror(&err));
 *       return EXIT_FAILURE;
 *   }
 */
int sconf_node_dict_insert(const char *name, struct SConfNode *parent,
                           struct SConfNode *node, struct SConfErr *err);

/**
 * Search for node in dictinary.
 *
 * Example:
 *   struct SConfNode *node;
 *   int r = sconf_node_dict_search("foo", dict, &node, &err);
 *   if (r == -1) {
 *       printf("Error: %s\n", sconf_strerror(&err));
 *       return EXIT_FAILURE;
 *   }
 */
int sconf_node_dict_search(const char *name, struct SConfNode *parent,
                           struct SConfNode **node, struct SConfErr *err);

/**
 * Iterate over nodes in dictionary using a callback function.
 *
 * Example:
 *   int dict_iterator_cb(const unsigned char *name, struct SConfNode *node,
 *                        void *user, struct SConfErr *err)
 *   {
 *       printf("Name of node: %s\n", name);
 *       int *count = (int *)user;
 *       *count += 1;
 *       return 0;
 *   }
 *
 *   [...]
 *
 *   int count = 0;
 *   int r = sconf_node_dict_foreach(dict, &dict_iterator_cb, &count, &err);
 *   if (r == -1) {
 *       printf("Error: %s\n", sconf_strerror(&err));
 *       return EXIT_FAILURE;
 *   }
 *
 *   printf("Nodes in dictionary: %d\n", count);
 */
int sconf_node_dict_foreach(struct SConfNode *dict,
                            int (*cb)(const unsigned char *name,
                                      struct SConfNode *node,
                                      void *user, struct SConfErr *err),
                            void *user, struct SConfErr *err);

/**
 * Insert node into array.
 *
 * Example:
 *   struct SConfNode *node = sconf_node_create(SCONF_TYPE_STR, "foobar", &err);
 *   if (!node) {
 *       printf("Error: %s\n", sconf_strerror(&err));
 *       return EXIT_FAILURE;
 *   }
 *
 *   int r = sconf_node_array_insert(0, array, node, &err);
 *   if (r == -1) {
 *       printf("Error: %s\n", sconf_strerror(&err));
 *       return EXIT_FAILURE;
 *   }
 */
int sconf_node_array_insert(uint32_t index, struct SConfNode *parent,
                            struct SConfNode *node, struct SConfErr *err);

/**
 * Search for node in array.
 *
 * Example:
 *   struct SConfNode *node;
 *   int r = sconf_node_array_search(0, array, &node, &err);
 *   if (r == -1) {
 *       printf("Error: %s\n", sconf_strerror(&err));
 *       return EXIT_FAILURE;
 *   }
 */
int sconf_node_array_search(uint32_t index, struct SConfNode *parent,
                            struct SConfNode **node, struct SConfErr *err);

/**
 * Iterate over nodes in array.
 *
 * Example:
 *   int count = 0;
 *   int r;
 *   struct SConfNode *node;
 *   uint32_t next = 0;
 *   while ((r = sconf_node_array_next(array, &node, &next, &err)) == 1)
 *   {
 *       count++;
 *   };
 *
 *   if (r == -1) {
 *       printf("Error: %s\n", sconf_strerror(&err));
 *       return EXIT_FAILURE;
 *   }
 *
 *   printf("Nodes in array: %d\n", count);
 */
int sconf_node_array_next(struct SConfNode *array, struct SConfNode **node,
                          uint32_t *next, struct SConfErr *err);

/**
 * Iterate over nodes in array using a callback function.
 *
 * Example:
 *   int array_iterator_cb(uint32_t index, struct SConfNode *node, void *user,
 *                         struct SConfErr *err)
 *   {
 *       int *count = (int *)user;
 *       *count += 1;
 *       return 0;
 *   }
 *
 *   [...]
 *
 *   int count = 0;
 *
 *   int r = sconf_node_array_foreach(array, &array_iterator_cb, &count, &err);
 *   if (r == -1) {
 *       printf("Error: %s\n", sconf_strerror(&err));
 *       return EXIT_FAILURE;
 *   }
 *
 *   printf("Nodes in array: %d\n", count);
 */
int sconf_node_array_foreach(struct SConfNode *array,
                              int (*cb)(uint32_t index, struct SConfNode *node,
                                        void *user, struct SConfErr *err),
                              void *user, struct SConfErr *err);

/**
 * Create config node if it does not exist and insert in parent.
 *
 * This is a shorthand for creating the node and then inserting it in to a
 * dictionary or array.
 *
 * Example (insert into array):
 *   struct SConfNode *n = sconf_node_create_and_insert(NULL, SCONF_TYPE_STR,
 *                                                      array, 2, "foobar",
                                                        err);
 *   if (!n) {
 *       printf("Error: %s\n", sconf_strerror(&err));
 *       return EXIT_FAILURE;
 *   }
 *
 * Example (insert into dictionary):
 *   struct SConfNode *n = sconf_node_create_and_insert("foo", SCONF_TYPE_STR,
 *                                                      dict, 0, "foobar", err);
 *   if (!n) {
 *       printf("Error: %s\n", sconf_strerror(&err));
 *       return EXIT_FAILURE;
 *   }
 */
struct SConfNode *sconf_node_create_and_insert(const char *name, uint8_t type,
                                               struct SConfNode *parent,
                                               uint32_t index, void *data,
                                               struct SConfErr *err);

/**
 * Read YAML file.
 *
 * Example:
 *   int r = sconf_yaml_read(root, "/etc/app.yaml", &err);
 *   if (r == -1) {
 *       printf("Error: %s\n", sconf_strerror(&err));
 *       return EXIT_FAILURE;
 *   }
 */
int sconf_yaml_read(struct SConfNode *root, const char *filename,
                    struct SConfErr *err);

/**
 * Parse command-line arguments.
 *
 * Example:
 *   struct SConfMap map[] = {
 *       {
 *           .path = "config_file",
 *           .type = SCONF_TYPE_YAML_FILE,
 *           .opts_short = 'c',
 *           .opts_long = "config-file",
 *           .help = "config file (YAML)",
 *       },
 *       {
 *           .type = SCONF_TYPE_USAGE,
 *           .opts_short = 'h',
 *           .opts_long = "help",
 *           .help = "print this help",
 *       },
 *       {0}
 *   };
 *
 *   int r = sconf_opts_parse(root, map, argc, argv, NULL, &err);
 *   if (r == -1) {
 *       printf("Error: %s\n", sconf_strerror(&err));
 *       return EXIT_FAILURE;
 *   }
 */
int sconf_opts_parse(struct SConfNode *root, const struct SConfMap *map,
                     int argc, char **argv, void *user,
                     struct SConfErr *err);

/**
 * Apply default values.
 *
 * Example:
 *   struct SConfMap map[] = {
 *       {
 *           .path = "foo.bar",
 *           .type = SCONF_TYPE_STR,
 *           .default_value = "barfoo",
 *       },
 *       {
 *           .path = "pi",
 *           .type = SCONF_TYPE_FLOAT,
 *           .default_value = "3.14",
 *       },
 *       {
 *           .path = "answer",
 *           .type = SCONF_TYPE_INT,
 *           .default_value = "42",
 *       },
 *       {0}
 *   };
 *
 *   int r = sconf_defaults(root, map, &err);
 *   if (r == -1) {
 *       printf("Error: %s\n", sconf_strerror(&err));
 *       return EXIT_FAILURE;
 *   }
 */
int sconf_defaults(struct SConfNode *root, const struct SConfMap *map,
                   struct SConfErr *err);

/**
 * Validate config.
 *
 * Example:
 *   int validate_stuff(const char *path, struct SConfNode *node, void *user,
 *                      struct SConfErr *err)
 *   {
 *       if (node && sconf_true(node)) {
 *           return 0;
 *       }
 *
 *       sconf_err_set(err, "stuff failed");
 *       return -1;
 *   }
 *
 *   [...]
 *
 *   struct SConfMap map[] = {
 *       {
 *           .path = "meh",
 *           .type = SCONF_TYPE_STR,
 *           .required = true,
 *       },
 *       {
 *           .path = "rofl",
 *           .type = SCONF_TYPE_BOOL,
 *           .validate_func = &validate_stuff
 *       },
 *       {0}
 *   };
 *
 *   int r = sconf_validate(root, map, NULL, &err);
 *   if (r == -1) {
 *       printf("Error: %s\n", sconf_strerror(&err));
 *       return EXIT_FAILURE;
 *   }
 */
int sconf_validate(struct SConfNode *root, const struct SConfMap *map,
                   void *user, struct SConfErr *err);

/**
 * Read environment variables.
 *
 * Example:
 *   struct SConfMap map[] = {
 *       {
 *           .path = "tro.lo.lo",
 *           .type = SCONF_TYPE_STR,
 *           .env = "TROLOLO",
 *       },
 *       {
 *           .path = "logging.enable",
 *           .type = SCONF_TYPE_BOOL,
 *           .env = "LOGGING_ENABLE",
 *       },
 *       {0}
 *   };
 *
 *   int r = sconf_env_read(root, map, &err);
 *   if (r == -1) {
 *       printf("Error: %s\n", sconf_strerror(&err));
 *       return EXIT_FAILURE;
 *   }
 */
int sconf_env_read(struct SConfNode *root, const struct SConfMap *map,
                   struct SConfErr *err);

/**
 * Do all the stuff! Option parsing, environment variables, defaults
 * and validation of config.
 *
 * Example:
 *   See examples/minimal.c for a example of how sconf_initialize works.
 */
int sconf_initialize(struct SConfNode *root, const struct SConfMap *map,
                     int argc, char **argv, void *user, struct SConfErr *err);

/**
 * Set error message.
 *
 * Example:
 *   struct SConfErr err = {0};
 *   sconf_err_set(&err, "this is a error message");
 */
void sconf_err_set(struct SConfErr *err, const char *fmt, ...);

/**
 * Get error message.
 *
 * Example:
 *   printf("Error: %s\n", sconf_strerror(&err);
 */
const char *sconf_strerror(struct SConfErr *err);

