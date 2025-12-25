# HSnow

*`xsnow` in Wayland.*

## Showcase

TODO.

## Installation

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
* `xxd` (required only when compiling from source; not needed when running the binary—see [Shaders](#shaders))

## Shaders

Shaders are located in the [`./shaders`](./shaders) directory. At compile time, `cmake` runs `xxd` on all shader files in this directory, producing variables of the form:

* `unsigned char shaders_{name}_{type}[]`
* `unsigned char shaders_{name}_{type}_len`

These variables are declared as `extern` in [`src/shader.h`](./src/shader.h) and included in [`src/shader.c`](./src/shader.c) so that they are correctly linked at build time.

Currently, `hsnow` does not support loading shaders at runtime—but this is a feature I’d like to add in the future.

## Roadmap

* [ ] X11 support
* [ ] Support for ALSA, FIFO, and other Linux audio backends
* [ ] Mouse and keyboard interactions
* [ ] Multi-monitor support

## References

* [`u/danihek`'s Post on Reddit](https://www.reddit.com/r/unixporn/comments/1pg59pl/oc_quick_snowy_app_for_wayland_i_wrote/)
* [The Wayland Protocol](https://wayland-book.com/)
* [Nix](https://nixos.org/)
