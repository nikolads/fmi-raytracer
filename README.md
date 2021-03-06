# FMI Raytracer

University project to make a GPU raytracer.

This is a work in progress. Currently it can only render a set of spheres hardcoded in the shader.
In addition most of the parameters for Vulkan are hardcoded and picked just so that it runs on the hardware I am testing on.

## How does it work

A compute shader is used to trace a ray for each pixel. The result is stored in an image, which is later copied to the framebuffer to be presented to the screen.

List of features:
- [x] geometry: sphere
- [ ] geometry: triangle mesh
- [x] material: opaque object
- [ ] material: transparent object
- [x] light: point light
- [ ] light: rectangle light

## Dependencies

- `Vulkan` and `Vulkan.hpp` - graphics and compute
- `Glfw3` - window creation
- `glsllangValidator` - compiler from GLSL to SPIR-V

## Building

This project comes with an adhoc `make` + `cmake` build script, which will only work with `clang` or `g++` because of hardcoded compiler flags and will probably only work on Linux.

- install `vulakn`, `glfw3` and `glsllangValidator` system-wide. `Vulkan.hpp` is automatically downloaded by the makefile.

- build with one of
    ```sh
    make build
    make build PROFILE=release
    ```

- or build and run with one of
    ```sh
    make run
    make run PROFILE=release
    ```

I may fix it someday if I find an adequate build system for C++.
