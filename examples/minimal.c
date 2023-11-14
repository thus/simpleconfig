#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

/* Only this header is needed by sconf itself! */
#include <sconf.h>

int validate_log_dir(const char *path, const struct SConfNode *node, void *data,
                     struct SConfErr *err)
{
    int rc = 0;

    DIR *dir = opendir(sconf_str(node));
    if (!dir) {
        sconf_err_set(err, "Could not use log directory '%s': %s",
                      sconf_str(node), strerror(errno));
        rc = -1;
    }

    closedir(dir);

    return rc;
}

static struct SConfMap map[] = {
    {
        .path = "config_file",
        .type = SCONF_TYPE_YAML_FILE,
        .opts_short = 'c',
        .opts_long = "config-file",
        .help = "config file (YAML)",
        .env = "APP_CONFIG_FILE",
    },
    {
        .path = "log.dir",
        .type = SCONF_TYPE_STR,
        .opts_short = 'l',
        .opts_long = "log-dir",
        .help = "log directory",
        .arg_type = "<dir>",
        .env = "APP_LOG_DIR",
        .default_value = "/var/log/app",
        .validate_func = &validate_log_dir,
    },
    {
        .path = "daemonize",
        .type = SCONF_TYPE_BOOL,
        .opts_short = 'D',
        .opts_long = "daemonize",
        .help = "run application in background",
        .default_value = "false",
    },
    {
        .type = SCONF_TYPE_USAGE,
        .opts_short = 'h',
        .opts_long = "help",
        .help = "print this help",
        .usage_desc = "All your base are belong to us."
    },
    {0}
};

int main(int argc, char **argv)
{
    struct SConfErr err = {0};

    struct SConfNode *root = SCONF_ROOT(&err);
    if (!root) {
        fprintf(stderr, "Error creating root node\n");
        return 1;
    }

    if (sconf_initialize(root, map, argc, argv, NULL, &err) == -1) {
        fprintf(stderr, "Error: %s\n", sconf_strerror(&err));
        sconf_node_destroy(root);
        return 1;
    }

    const char *log_dir;
    int r = sconf_get_str(root, "log.dir", &log_dir, &err);
    if (r == -1) {
        fprintf(stderr, "Error getting log.dir: %s\n", sconf_strerror(&err));
        sconf_node_destroy(root);
        return 1;
    }
    else if (r == 1) {
        printf("Log directory is '%s'\n", log_dir);
    }

    sconf_node_destroy(root);

    return 0;
}

