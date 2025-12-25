#include "config.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

AppConfig config = {
    .num_layers = 33u,
    .depth = 1.5,
    .width = 0.4,
    .speed = 0.6,
    .alpha = 0.1,
    .surface_layer = SURFACE_LAYER_TOP,
};

const char *get_config_path(void)
{
    static char path[PATH_MAX];

    const char *xdg = getenv("XDG_CONFIG_HOME");
    const char *home = getenv("HOME");

    if (xdg && xdg[0])
    {
        snprintf(path, sizeof(path), "%s/hsnow/config", xdg);
    }
    else if (home && home[0])
    {
        snprintf(path, sizeof(path), "%s/.config/hsnow/config", home);
    }
    else
    {
        return NULL; // no usable config dir
    }

    return path;
}

void load_config(AppConfig *cfg, const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f)
        return;

    char line[256];

    while (fgets(line, sizeof(line), f))
    {
        char *eq, *key, *val;

        if ((eq = strchr(line, '#')))
            *eq = '\0';

        eq = strchr(line, '=');
        if (!eq)
            continue;

        *eq = '\0';
        key = strtok(line, " \t\n");
        val = strtok(eq + 1, " \t\n");

        if (!key || !val)
            continue;

        if (strcmp(key, "num_layers") == 0)
        {
            cfg->num_layers = (size_t)atoi(val);
        }
        else if (strcmp(key, "depth") == 0)
        {
            cfg->depth = strtof(val, NULL);
        }
        else if (strcmp(key, "width") == 0)
        {
            cfg->width = strtof(val, NULL);
        }
        else if (strcmp(key, "speed") == 0)
        {
            cfg->speed = strtof(val, NULL);
        }
        else if (strcmp(key, "alpha") == 0)
        {
            cfg->alpha = strtof(val, NULL);
        }
        else if (!strcmp(key, "surface_layer"))
        {
            if (!strcmp(val, "background"))
                cfg->surface_layer = SURFACE_LAYER_BACKGROUND;
            else if (!strcmp(val, "bottom"))
                cfg->surface_layer = SURFACE_LAYER_BOTTOM;
            else if (!strcmp(val, "top"))
                cfg->surface_layer = SURFACE_LAYER_TOP;
            else if (!strcmp(val, "overlay"))
                cfg->surface_layer = SURFACE_LAYER_OVERLAY;
        }
    }

    fclose(f);
}
