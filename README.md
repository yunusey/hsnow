# HSnow

*`xsnow` in Wayland.*

## Showcase

TODO.

## Shaders

Shaders are located in the [`./shaders`](./shaders) directory. At compile time, `cmake` runs `xxd` on all shader files in this directory, producing variables of the form:

* `unsigned char shaders_{name}_{type}[]`
* `unsigned char shaders_{name}_{type}_len`

These variables are declared as `extern` in [`src/shader.h`](./src/shader.h) and included in [`src/shader.c`](./src/shader.c) so that they are correctly linked at build time.

Currently, `hsnow` does not support loading shaders at runtime—but this is a feature I'd like to add in the future.

## Build Options

When building with CMake, you can configure the following options:

* **`ENABLE_VSYNC`** (default: `ON`): Enable vertical synchronization to cap frame rate at your monitor's refresh rate (typically 60 FPS). Disable this to unlock higher frame rates.
  ```sh
  cmake -DENABLE_VSYNC=OFF ..
  ```

* **`ENABLE_ASAN`** (default: `ON`): Enable AddressSanitizer in Debug builds for memory error detection.

Additionally, you can modify the target FPS by changing the `TARGET_FPS` macro in `src/main.c` (default: 120). Note that VSync must be disabled to achieve frame rates higher than your monitor's refresh rate.

## Roadmapion

### Nix

If you're using the [Nix](https://nixos.org/) package manager, you can run the project with:

```sh
nix run github:yunusey/hsnow
```

### Other Distributions

`hsnow` runs exclusively on Linux. You will need the following dependencies:

* `libffi`
* `libGLU`
* `egl-wayland`
* `libxkbcommon`
* `xxd` (required only when compiling from source; not needed when running the binary--see [Shaders](#shaders))

## Configuration

To override the defaults, you need to paste the following content to `$XDG_CONFIG_HOME/hsnow/config`:

```conf
# Non-negative integer controlling the number of layers
num_layers = 33

# The depth of layers
depth = 1.5

# The speed of, well, the snow :D
speed = 0.4

# The wideness of individual snow flakes
width = 0.6

# The alpha channel of the overall shader; applied to the final color
alpha = 0.1

# Available values are `background`, `bottom`, `overlay`, and `top`
surface_layer = top
```

## Shaders

Shaders are located in the [`./shaders`](./shaders) directory. At compile time, `cmake` runs `xxd` on all shader files in this directory, producing variables of the form:

* `unsigned char shaders_{name}_{type}[]`
* `unsigned char shaders_{name}_{type}_len`

These variables are declared as `extern` in [`src/shader.h`](./src/shader.h) and included in [`src/shader.c`](./src/shader.c) so that they are correctly linked at build time.

Currently, `hsnow` does not support loading shaders at runtime—but this is a feature I’d like to add in the future.

## Roadmap

* [ ] X11 support?
* [ ] Mouse and keyboard interactions
* [ ] Multi-monitor support

## References

* [`u/danihek`'s Post on Reddit](https://www.reddit.com/r/unixporn/comments/1pg59pl/oc_quick_snowy_app_for_wayland_i_wrote/)
* [The Wayland Protocol](https://wayland-book.com/)
* [Nix](https://nixos.org/)
