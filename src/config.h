#include <stdbool.h>
#include <stddef.h>

// NOTE: An abstract representation of surface layers--not using `zwlr_layer_shell_v1_layer` enum directly to avoid
// coupling config to Wayland specifics.
typedef enum
{
    SURFACE_LAYER_BACKGROUND,
    SURFACE_LAYER_BOTTOM,
    SURFACE_LAYER_TOP,
    SURFACE_LAYER_OVERLAY
} SurfaceLayer;

typedef struct
{
    size_t num_layers;
    float depth;
    float width;
    float speed;
    float alpha;
    SurfaceLayer surface_layer;
} AppConfig;

extern AppConfig config;

void load_config(AppConfig *cfg, const char *path);
const char *get_config_path();
