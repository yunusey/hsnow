#include "platform.h"
#include "wlr-layer-shell-client-protocol.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GL/gl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wayland-client.h>
#include <wayland-cursor.h>
#include <wayland-egl.h>
#include <xkbcommon/xkbcommon.h>

PlatformData platform = {0};
CoreData core = {0};

static void handle_surface_enter(void *data, struct wl_surface *surface, struct wl_output *output)
{
    for (int i = 0; i < platform.monitorCount; i++)
    {
        if (platform.monitors[i].output == output)
        {
            platform.currentMonitorIndex = i;
            return;
        }
    }
}
static void handle_surface_leave(void *data, struct wl_surface *surface, struct wl_output *output)
{
    for (int i = 0; i < platform.monitorCount; i++)
    {
        if (platform.monitors[i].output == output)
        {
            platform.currentMonitorIndex = -1;
            return;
        }
    }
}
static const struct wl_surface_listener surface_listener = {
    .enter = &handle_surface_enter,
    .leave = &handle_surface_leave,
};

static void layer_surface_configure(void *data, struct zwlr_layer_surface_v1 *zwlr_layer_surface_v1, uint32_t serial,
                                    uint32_t width, uint32_t height)
{
    core.window_size.width = width;
    core.window_size.height = height;

    zwlr_layer_surface_v1_ack_configure(platform.layer_surface, serial);
    wl_egl_window_resize(platform.egl_window, width, height, 0, 0);
    eglMakeCurrent(platform.egl.device, platform.egl.surface, platform.egl.surface, platform.egl.context);
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, 0, height, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}
static void layer_surface_closed(void *data, struct zwlr_layer_surface_v1 *zwlr_layer_surface_v1)
{
}

static const struct zwlr_layer_surface_v1_listener layer_surface_listener = {
    .configure = &layer_surface_configure,
    .closed = &layer_surface_closed,
};

static void handle_output_mode(void *data, struct wl_output *output, uint32_t flags, int32_t width, int32_t height,
                               int32_t refresh)
{
    MonitorData *monitor = data;
    if (flags & WL_OUTPUT_MODE_CURRENT)
    {
        monitor->width = width;
        monitor->height = height;
        monitor->refresh = refresh;
    }
}
static void handle_output_scale(void *data, struct wl_output *output, int32_t factor)
{
    MonitorData *monitor = data;
    monitor->scale = factor;
}
static void handle_output_geometry(void *data, struct wl_output *output, int32_t x, int32_t y, int32_t physical_width,
                                   int32_t physical_height, int32_t subpixel, const char *make, const char *model,
                                   int32_t transform)
{
}
static void handle_output_done(void *data, struct wl_output *output)
{
}
static void handle_output_name(void *data, struct wl_output *output, const char *name)
{
}
static void handle_output_description(void *data, struct wl_output *output, const char *description)
{
}

static const struct wl_output_listener output_listener = {
    .mode = handle_output_mode,
    .scale = handle_output_scale,
    .geometry = handle_output_geometry,
    .done = handle_output_done,
    .name = handle_output_name,
    .description = handle_output_description,
};

static void registry_handle_global(void *data, struct wl_registry *registry, uint32_t name, const char *interface,
                                   uint32_t version)
{
    if (strcmp(interface, wl_compositor_interface.name) == 0)
    {
        platform.compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 4);
    }
    else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0)
    {
        platform.layer_shell = wl_registry_bind(registry, name, &zwlr_layer_shell_v1_interface, 1);
    }
    else if (strcmp(interface, wl_output_interface.name) == 0)
    {
        if (platform.monitorCount < MAX_MONITORS)
        {
            MonitorData *monitor = &platform.monitors[platform.monitorCount];
            monitor->output = wl_registry_bind(registry, name, &wl_output_interface, 4);
            wl_output_add_listener(monitor->output, &output_listener, monitor);
            platform.monitorCount++;
        }
        else
        {
        }
    }
}
static void registry_handle_global_remove(void *data, struct wl_registry *registry, uint32_t name)
{
}

static const struct wl_registry_listener registry_listener = {
    .global = registry_handle_global,
    .global_remove = registry_handle_global_remove,
};

bool init_platform(const AppConfig *config)
{
    const EGLint framebufferAttribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8,
        EGL_NONE};
    const EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};

    EGLint numConfigs = 0;

    platform.display = wl_display_connect(NULL);

    platform.registry = wl_display_get_registry(platform.display);
    wl_registry_add_listener(platform.registry, &registry_listener, NULL);
    wl_display_roundtrip(platform.display);

    platform.egl.device = eglGetDisplay((EGLNativeDisplayType)platform.display);

    if (eglInitialize(platform.egl.device, NULL, NULL) == EGL_FALSE)
    {
        return false;
    }
    if (eglBindAPI(EGL_OPENGL_API) == EGL_FALSE)
    {
        return false;
    }
    if (eglChooseConfig(platform.egl.device, framebufferAttribs, &platform.egl.config, 1, &numConfigs) == EGL_FALSE ||
        numConfigs == 0)
    {
        return false;
    }
    platform.egl.context = eglCreateContext(platform.egl.device, platform.egl.config, EGL_NO_CONTEXT, contextAttribs);
    if (platform.egl.context == EGL_NO_CONTEXT)
    {
        return -1;
    }

    platform.surface = wl_compositor_create_surface(platform.compositor);

    // We don't want any input region for now
    struct wl_region *empty_region = wl_compositor_create_region(platform.compositor);
    wl_surface_set_input_region(platform.surface, empty_region);
    wl_region_destroy(empty_region);

    wl_surface_add_listener(platform.surface, &surface_listener, NULL);
    if (platform.surface == NULL)
    {
        return -1;
    }

    enum zwlr_layer_shell_v1_layer layer;
    switch (config->surface_layer)
    {
    case SURFACE_LAYER_BACKGROUND:
        layer = ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND;
        break;
    case SURFACE_LAYER_BOTTOM:
        layer = ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM;
        break;
    case SURFACE_LAYER_TOP:
        layer = ZWLR_LAYER_SHELL_V1_LAYER_TOP;
        break;
    case SURFACE_LAYER_OVERLAY:
        layer = ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY;
        break;
    default:
        layer = ZWLR_LAYER_SHELL_V1_LAYER_TOP;
        break;
    }
    platform.layer_surface =
        zwlr_layer_shell_v1_get_layer_surface(platform.layer_shell, platform.surface, NULL, layer, "wayland_app");

    zwlr_layer_surface_v1_set_size(platform.layer_surface, 0, 0);
    zwlr_layer_surface_v1_set_anchor(platform.layer_surface,
                                     ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP | ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM |
                                         ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT | ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT);
    zwlr_layer_surface_v1_add_listener(platform.layer_surface, &layer_surface_listener, &platform);
    zwlr_layer_surface_v1_set_keyboard_interactivity(platform.layer_surface,
                                                     ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_NONE);

    platform.egl_window =
        wl_egl_window_create(platform.surface, 800, 600); // Initial size; will be resized on configure
    platform.egl.surface = eglCreateWindowSurface(platform.egl.device, platform.egl.config,
                                                  (EGLNativeWindowType)platform.egl_window, NULL);
    wl_surface_commit(platform.surface);
    wl_display_roundtrip(platform.display);

#ifdef ENABLE_VSYNC
    eglSwapInterval(platform.egl.device, ENABLE_VSYNC ? 1 : 0);
#else
    eglSwapInterval(platform.egl.device, 0);
#endif

    EGLBoolean result =
        eglMakeCurrent(platform.egl.device, platform.egl.surface, platform.egl.surface, platform.egl.context);
    if (result != EGL_FALSE)
    {
    }
    else
    {
        return -1;
    }

    return 0;
}

void begin_drawing(void)
{
    glViewport(0, 0, core.window_size.width, core.window_size.height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, core.window_size.width, 0, core.window_size.height, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void end_drawing(void)
{
    swap_screen_buffers();
    poll_input_events();
}

void poll_input_events(void)
{
    wl_display_dispatch_pending(platform.display);
    wl_display_flush(platform.display);
}

void swap_screen_buffers(void)
{
    eglSwapBuffers(platform.egl.device, platform.egl.surface);
}

bool close_platform(void)
{
    if (platform.layer_surface)
        zwlr_layer_surface_v1_destroy(platform.layer_surface);

    if (platform.egl.device)
    {
        eglMakeCurrent(platform.egl.device, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

        if (platform.egl.surface)
            eglDestroySurface(platform.egl.device, platform.egl.surface);

        if (platform.egl.context)
            eglDestroyContext(platform.egl.device, platform.egl.context);

        eglTerminate(platform.egl.device);
    }

    if (platform.egl_window)
        wl_egl_window_destroy(platform.egl_window);

    if (platform.surface)
        wl_surface_destroy(platform.surface);

    if (platform.layer_shell)
        zwlr_layer_shell_v1_destroy(platform.layer_shell);
    if (platform.compositor)
        wl_compositor_destroy(platform.compositor);
    if (platform.registry)
        wl_registry_destroy(platform.registry);

    if (platform.display)
    {
        wl_display_flush(platform.display);
        wl_display_disconnect(platform.display);
    }

    return true;
}
