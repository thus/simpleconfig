#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <yaml.h>

#include "convert.h"
#include "sconf_private.h"

enum {
    SCONF_YAML_STATE_START,
    SCONF_YAML_STATE_STOP,
    SCONF_YAML_STATE_STREAM,
    SCONF_YAML_STATE_DOCUMENT,
    SCONF_YAML_STATE_BLOCK,
    SCONF_YAML_STATE_BLOCK_CONTENT,
    SCONF_YAML_STATE_SEQUENCE,
};

struct SConfYAMLParent {
    struct SConfNode *parent;
    uint32_t curr_index;
    struct SConfYAMLParent *next;
};

struct SConfYAMLState {
    uint8_t state;
    int depth;
    char *curr_key;
    struct SConfYAMLParent *curr_parent;
};

/**
 * @internal
 * @brief Push parent onto parent stack.
 *
 * @param parent Pointer to parent.
 * @param state  State of the YAML parser.
 * @param err    Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
static int sconf_yaml_parent_stack_push(struct SConfNode *parent,
                                        struct SConfYAMLState *state,
                                        struct SConfErr *err)
{
    assert(parent);
    assert(state);

    if (state->depth >= SCONF_MAX_DEPTH - 1) {
        sconf_err_set(err, "maximum depth reached when reading YAML file");
        return -1;
    }
    state->depth++;

    struct SConfYAMLParent *p = calloc(1, sizeof(struct SConfYAMLParent));
    if (!p) {
        sconf_err_set(err, "failed to allocate memory for YAML parent");
        return -1;
    }

    p->parent = parent;
    p->next = state->curr_parent;
    state->curr_parent = p;

    return 0;
}

/**
 * @internal
 * @brief Pop parent from parent stack.
 *
 * @param state State of the YAML parser.
 * @param err   Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
static int sconf_yaml_parent_stack_pop(struct SConfYAMLState *state,
                                       struct SConfErr *err)
{
    assert(state);

    struct SConfYAMLParent *p = state->curr_parent;

    if (!p) {
        sconf_err_set(err, "parent stack is empty");
        return -1;
    }

    state->depth--;
    state->curr_parent = p->next;

    free(p);

    return 0;
}

/**
 * @internal
 * @brief Destroy YAML parsing state.
 *
 * @param state State of the YAML parser.
 */
static void sconf_yaml_state_destroy(struct SConfYAMLState *state)
{
    assert(state);

    while (state->curr_parent)
    {
        sconf_yaml_parent_stack_pop(state, NULL);
    }

    if (state->curr_key) {
        free(state->curr_key);
    }
}

/**
 * @internal
 * @brief Add parent to YAML state.
 *
 * @param state State of the YAML parser.
 * @param type  Type of parent to add.
 * @param err   Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
static int sconf_yaml_parent_add(struct SConfYAMLState *state, uint8_t type,
                                 struct SConfErr *err)
{
    assert(state);
    assert(type == SCONF_TYPE_DICT || type == SCONF_TYPE_ARRAY);

    struct SConfYAMLParent *p = state->curr_parent;
    if (!p) {
        sconf_err_set(err, "parent stack is empty");
        return -1;
    }

    if (p->parent->type == SCONF_TYPE_DICT && !state->curr_key) {
        sconf_err_set(err, "parent is dict, but key is not set");
        return -1;
    }

    struct SConfNode *parent = sconf_node_create_and_insert(state->curr_key,
                                                     type,
                                                     p->parent, p->curr_index,
                                                     NULL, err);
    if (parent == NULL) {
        return -1;
    }

    if (sconf_yaml_parent_stack_push(parent, state, err) != 0) {
        return -1;
    }

    if (p->parent->type == SCONF_TYPE_ARRAY) {
        p->curr_index++;
    }
    else if (p->parent->type == SCONF_TYPE_DICT) {
        free(state->curr_key);
        state->curr_key = NULL;
    }

    return 0;
}

/**
 * @internal
 * @brief Remove parent from YAML state.
 *
 * @param state State of the YAML parser.
 * @param type  Expected type of parent to remove.
 * @param err   Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
static int sconf_yaml_parent_remove(struct SConfYAMLState *state, uint8_t type,
                                    struct SConfErr *err)
{
    assert(state);
    assert(type == SCONF_TYPE_DICT || type == SCONF_TYPE_ARRAY);

    struct SConfYAMLParent *p = state->curr_parent;
    if (!p) {
        sconf_err_set(err, "parent stack is empty");
        return -1;
    }

    if (sconf_yaml_parent_stack_pop(state, err) != 0) {
        return -1;
    }

    return 0;
}
#pragma GCC diagnostic pop

/**
 * @internal
 * @brief Add node to config.
 *
 * @param state       State of the YAML parser.
 * @param parent_type Expected parent type.
 * @param event       The YAML event.
 * @param err         Pointer to error struct.
 */
static int sconf_yaml_node_add(struct SConfYAMLState *state,
                               uint8_t parent_type, yaml_event_t *event,
                               struct SConfErr *err)
{
    assert(state);
    assert(event);
    assert(parent_type == SCONF_TYPE_DICT || parent_type == SCONF_TYPE_ARRAY);

    struct SConfYAMLParent *p = state->curr_parent;
    if (!p) {
        sconf_err_set(err, "parent stack is empty");
        return -1;
    }

    char *value_str = (char *)event->data.scalar.value;

    if (parent_type != p->parent->type) {
        sconf_err_set(err, "expected parent type '%d' but got '%d'",
                      parent_type, p->parent->type);
        return -1;
    }

    void *data = value_str;
    uint8_t type = SCONF_TYPE_STR;
    int64_t integer;
    bool boolean;
    double fp;
    int r = 0;

    if (event->data.scalar.style == YAML_DOUBLE_QUOTED_SCALAR_STYLE ||
            event->data.scalar.style == YAML_SINGLE_QUOTED_SCALAR_STYLE ||
            strlen(value_str) == 0) {
        data = value_str;
        type = SCONF_TYPE_STR;
    }
    else if ((r = sconf_string_to_integer(value_str, &integer, err))) {
        data = &integer;
        type = SCONF_TYPE_INT;
    }
    else if ((r = sconf_string_to_float(value_str, &fp, err))) {
        data = &fp;
        type = SCONF_TYPE_FLOAT;
    }
    else if ((r = sconf_string_to_bool(value_str, &boolean))) {
        data = &boolean;
        type = SCONF_TYPE_BOOL;
    }

    if (r == -1) {
        return -1;
    }

    struct SConfNode *node = sconf_node_create_and_insert(state->curr_key, type,
                                                          p->parent,
                                                          p->curr_index, data,
                                                          err);
    if (!node) {
        return -1;
    }

    if (parent_type == SCONF_TYPE_ARRAY) {
        p->curr_index++;
    }
    else if (parent_type == SCONF_TYPE_DICT) {
        free(state->curr_key);
        state->curr_key = NULL;
    }
 
    return 0;
}

/**
 * @internal
 * @brief Consume event from YAML parser.
 *
 * FUTURE: it would make more sense to implement this as a recursive
 * decent parser. Then all the parent stack code could be removed.
 *
 * @param root  The config node root.
 * @param event The YAML event.
 * @param state State of the YAML parser.
 * @param err   Pointer to error struct.
 *
 * @return 1 on success, 0 otherwise.
 */
static int sconf_yaml_consume_event(struct SConfNode *root, yaml_event_t *event,
                                    struct SConfYAMLState *state,
                                    struct SConfErr *err)
{
    assert(root);
    assert(event);
    assert(state);

    switch (state->state)
    {
        case SCONF_YAML_STATE_START:
            switch (event->type)
            {
                case YAML_STREAM_START_EVENT:
                    state->state = SCONF_YAML_STATE_STREAM;
                    break;

                default:
                    sconf_err_set(err, "Unexpected event %d in state %d",
                                  event->type, state->state);
                    return 0;
            } 
            break;

        case SCONF_YAML_STATE_STREAM:
            switch (event->type)
            {
                case YAML_DOCUMENT_START_EVENT:
                    state->state = SCONF_YAML_STATE_DOCUMENT;
                    break;

                case YAML_STREAM_END_EVENT:
                    state->state = SCONF_YAML_STATE_STOP;
                    break;

                default:
                    sconf_err_set(err, "Unexpected event %d in state %d",
                                  event->type, state->state);
                    return 0;
            }
            break;

        case SCONF_YAML_STATE_DOCUMENT:
            switch (event->type)
            {
                case YAML_MAPPING_START_EVENT:
                    if (sconf_yaml_parent_stack_push(root, state, err) == -1) {
                        return 0;
                    }
                    state->state = SCONF_YAML_STATE_BLOCK;
                    break;

                case YAML_DOCUMENT_END_EVENT:
                    state->state = SCONF_YAML_STATE_STREAM;
                    break;

                default:
                    sconf_err_set(err, "Unexpected event %d in state %d",
                                  event->type, state->state);
                    return 0;
            }
            break;

        case SCONF_YAML_STATE_BLOCK:
            switch (event->type)
            {
                case YAML_SCALAR_EVENT:
                    if (state->curr_key) {
                        sconf_err_set(err, "key is already set to '%s'",
                                      state->curr_key);
                        free(state->curr_key);
                        state->curr_key = NULL;
                        return 0;
                    }
                    state->curr_key = strdup((char *)event->data.scalar.value);
                    if (!state->curr_key) {
                        sconf_err_set(err, "could not allocate memory for key");
                        return 0;
                    }
                    state->state = SCONF_YAML_STATE_BLOCK_CONTENT;
                    break;

                case YAML_SEQUENCE_END_EVENT:
                    if (sconf_yaml_parent_remove(state, SCONF_TYPE_ARRAY,
                                                 err) != 0) {
                        return 0;
                    }
                    break;

                case YAML_MAPPING_END_EVENT:
                    if (sconf_yaml_parent_remove(state, SCONF_TYPE_DICT,
                                                 err) != 0) {
                        return 0;
                    }
                    if (state->curr_parent &&
                            state->curr_parent->parent->type == SCONF_TYPE_ARRAY) {
                        state->state = SCONF_YAML_STATE_SEQUENCE;
                    }
                    break;

                case YAML_MAPPING_START_EVENT:
                    if (sconf_yaml_parent_add(state, SCONF_TYPE_DICT,
                                              err) != 0) {
                        return 0;
                    }
                    break;

                case YAML_DOCUMENT_END_EVENT:
                    state->state = SCONF_YAML_STATE_STREAM;
                    break;

                default:
                    sconf_err_set(err, "Unexpected event %d in state %d",
                                  event->type, state->state);
                    return 0;
            }
            break;

        case SCONF_YAML_STATE_BLOCK_CONTENT:
            switch (event->type)
            {
                case YAML_SCALAR_EVENT:
                    if (sconf_yaml_node_add(state, SCONF_TYPE_DICT, event,
                                            err) != 0) {
                        return 0;
                    }
                    state->state = SCONF_YAML_STATE_BLOCK;
                    break;

                case YAML_MAPPING_START_EVENT:
                    if (sconf_yaml_parent_add(state, SCONF_TYPE_DICT,
                                              err) != 0) {
                        return 0;
                    }
                    state->state = SCONF_YAML_STATE_BLOCK;
                    break;

                case YAML_SEQUENCE_START_EVENT:
                    if (sconf_yaml_parent_add(state, SCONF_TYPE_ARRAY,
                                              err) != 0) {
                        return 0;
                    }
                    state->state = SCONF_YAML_STATE_SEQUENCE;
                    break;

                default:
                    sconf_err_set(err, "Unexpected event %d in state %d",
                                  event->type, state->state);
                    return 0;
            }
            break;

        case SCONF_YAML_STATE_SEQUENCE:
            switch (event->type)
            {
                case YAML_SCALAR_EVENT:
                    if (sconf_yaml_node_add(state, SCONF_TYPE_ARRAY, event,
                                            err) != 0) {
                        return 0;
                    }
                    break;

                case YAML_MAPPING_START_EVENT:
                    if (sconf_yaml_parent_add(state, SCONF_TYPE_DICT,
                                              err) != 0) {
                        return 0;
                    }
                    state->state = SCONF_YAML_STATE_BLOCK;
                    break;

                case YAML_SEQUENCE_START_EVENT:
                    if (sconf_yaml_parent_add(state, SCONF_TYPE_ARRAY,
                                              err) != 0) {
                        return 0;
                    }
                    state->state = SCONF_YAML_STATE_SEQUENCE;
                    break;

                case YAML_SEQUENCE_END_EVENT:
                    if (sconf_yaml_parent_remove(state, SCONF_TYPE_ARRAY,
                                                 err) != 0) {
                        return 0;
                    }
                    state->state = SCONF_YAML_STATE_BLOCK;
                    break;

                default:
                    sconf_err_set(err, "Unexpected event %d in state %d",
                                  event->type, state->state);
                    return 0;
            }
            break;
    }

    return 1;
}

/**
 * @brief Read config from YAML file.
 *
 * @param root     The config root node.
 * @param filename Path to YAML file to read.
 * @param err      Pointer to error struct.
 *
 * @return 0 on success, -1 otherwise.
 */
int sconf_yaml_read(struct SConfNode *root, const char *filename,
                    struct SConfErr *err)
{
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        sconf_err_set(err, "could not open file '%s': %s", filename,
                      strerror(errno));
        return -1;
    }

    struct SConfYAMLState state = {0};
    state.state = SCONF_YAML_STATE_START;

    int return_code = 0;

    yaml_parser_t parser;
    if (!yaml_parser_initialize(&parser)) {
        sconf_err_set(err, "failed to initialize YAML parser");
        return_code = -1;
        goto end;
    }

    yaml_parser_set_input_file(&parser, fp);

    do {
        yaml_event_t event;

        uint8_t success = yaml_parser_parse(&parser, &event);
        if (!success) {
            sconf_err_set(err, "error parsing YAML");
            return_code = -1;
            goto end;
        }

        success = sconf_yaml_consume_event(root, &event, &state, err);
        yaml_event_delete(&event);
        if (!success) {
            return_code = -1;
            goto end;
        }

    } while (state.state != SCONF_YAML_STATE_STOP);

end:
    sconf_yaml_state_destroy(&state);
    yaml_parser_delete(&parser);

    if (fclose(fp) == EOF) {
        sconf_err_set(err, "error closing file '%s': %s\n", filename,
                      strerror(errno));
        return_code = -1;
    }

    return return_code;
}

