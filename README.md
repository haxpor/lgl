# Build

To build demo programs, `cd` to `src/` directory, then use `make.sh` script to do it as follows.

`./make.sh build <target .cpp file>`

for example

`./make.sh build src/Shader.cpp`

it will produce `a.out` file at the root directory, run it with `./a.out`. Any resource it needs to
access is inside `data` directory.

# Dependencies

Included in `external/` directory

* `glad` - generated from its webservice for OpenGL 3.3 API, Core profile (subject to be gradually added more API support later if need)
* `stb_image` - for loading image asset

Others

* `glfw` - for window stuff
* `glm` - for OpenGL Mathematics

# make.sh

Suport the following commands

* `./make.sh build src/<main-source-file.cpp>` - to build target source file, provided that main function is in this file
* `./make.sh buildsc src/self-contained/<dir>/<main-source-file.cpp>` - to build target source file. It is based on FreeGLUT, offer more flat view on sample source code (currently lean towards this path)
* `./make.sh list` - list all available example programs to build
* `./make.sh help` - print help message

# Note

All common source files/headers are inside `src/core`.

Demo programs are a single source file for each, and located in `src/*.cpp`.

* At the near end of completion of study (thus this project), `CMake` build script will be added for convenience in building all demo programs. For now, it makes no sense to incur having it while studying. Modify `make.sh` as needed for your system to make it work for the time being.

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
