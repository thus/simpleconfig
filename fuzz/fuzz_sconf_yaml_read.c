#include <sconf.h>

int main(int argc, char **argv)
{
    struct SConfNode *root = SCONF_ROOT(NULL);
    if (!root) {
        return 1;
    }

    while (__AFL_LOOP(1000))
    {
        sconf_yaml_read(root, "/dev/stdin", NULL);
    }

    sconf_node_destroy(root);

    return 0;
} 

