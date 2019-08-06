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

At the near end of completion of study (thus this project), `CMake` build script will be added for convenience in building all demo programs. For now, it makes no sense to incur having it while studying. Modify `build.sh` as needed for your system to make it work for the time being.

# How to create a new demo program

See `src/Barebone.cpp` as example.

In short

1. Include `Base.h`
2. Inherit your custom class from `lgl::App` then implement relevant methods i.e. `UserUpdate()`, `UserRender()` etc as need.
3. Call `lgl::App`'s `Setup()`, and `Start()`.

You're done. Anyway to just quickly show an empty window, you can skip step 2, but that's not very useful right?

# License

All non-source asset from learnopengl.com, thus adhere to its original.
Source code is adapted from original under MIT.
