# Compile

Tested on Linux only (anyway should be slight effort to build it on Windows and macOS).

Use `./build.sh <demo-file>` to build in `src/` directory. Then execute it with `./a.out`.

# Dependencies

Included in `external/` directory

* `glad` - generated from its webservice for OpenGL 3.3 API, Core profile (subject to be gradually added more API support later if need)
* `stb_image` - for loading image asset

Others

* `glfw` - for window stuff

# Note

All common source files/headers are inside `src/core`.

Demo programs are a single source file for each, and located in `src/*.cpp`.

# How to create a new demo program

See `src/Barebone.cpp` as example.

In short

1. Include `Base.h`
2. Inherit your custom class from `lgl::App` then implement relevant methods i.e. `UserUpdate()`, `UserRender()` etc as need.
3. Call `lgl::App`'s `Setup()`, and `Start()`.

You're done. Anyway to just quickly show an empty window, you can skip step 2, but that's not very useful right?

# License

All non-source asset from learnopengl.com.
Source code is adapted from original.
