#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sconf.h>

int main(int argc, char **argv)
{
    struct SConfNode *root = SCONF_ROOT(NULL);
    if (!root) {
        return 1;
    }

    while (__AFL_LOOP(1000))
    {
        char input[4096] = {0};
        /* We want a string, so always leave the last byte as zero */
        if (read(STDIN_FILENO, input, 4095) < 0) {
            printf("Could not read stdin\n");
        }
    
        /* Fuzz path */
        sconf_set(root, input, SCONF_TYPE_STR, "foobar", NULL);
    
        struct SConfNode *node = NULL;
        sconf_get(root, input, &node, NULL);
    
        /* Fuzz value */
        sconf_set(root, "foo.str", SCONF_TYPE_STR, input, NULL);
        sconf_set(root, "foo.int", SCONF_TYPE_INT, input, NULL);
        sconf_set(root, "foo.bool", SCONF_TYPE_BOOL, input, NULL);
        sconf_set(root, "foo.float", SCONF_TYPE_FLOAT, input, NULL);
    }

    sconf_node_destroy(root);

    return 0;
}

