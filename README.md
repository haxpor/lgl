# Build

To build demo programs, `cd` to `src/` directory, then use `make.sh` script to do it as follows.

`./make.sh build <target .cpp file>`

for example

`./make.sh build Shader.cpp`

it will produce `a.out` file, run it with `./a.out`.

# make.sh

There is other functionality `make.sh` offers.

It's based on `./make.sh <command> <optional-parameters>`.

## List all demo programs

Use `./make.sh list` to list all demo programs along with its corresponding description.

## Help

Use `./make.sh help` to show help text.

# Dependencies

Included in `external/` directory

* `glad` - generated from its webservice for OpenGL 3.3 API, Core profile (subject to be gradually added more API support later if need)
* `stb_image` - for loading image asset

Others

* `glfw` - for window stuff

# Note

All common source files/headers are inside `src/core`.

Demo programs are a single source file for each, and located in `src/*.cpp`.

At the near end of completion of study (thus this project), `CMake` build script will be added for convenience in building all demo programs. For now, it makes no sense to incur having it while studying. Modify `make.sh` as needed for your system to make it work for the time being.

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
