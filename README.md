# Build

Some programs have their own separate `Makefile` in its directory, but most can be build using `make.sh`
script as follows.

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

* `./make.sh build src/<directory>/<main-source-file.cpp>` - to build target source file, provided that main function is in this file
* `./make.sh help` - print help message

# Note

All common source files/headers are inside `src/core`.

Demo programs are a single source file for each, and located in `src/*.cpp`.

* At the near end of completion of study (thus this project), `CMake` build script will be added for convenience in building all demo programs. For now, it makes no sense to incur having it while studying. Modify `make.sh` as needed for your system to make it work for the time being.
* **Beware**: Tested only on Linux, technically should work the same on other platforms but might need slight modification on build script, or particular code. Also some sample make uses of symlink to get access to `../../data` conveniently.

# License

All non-source asset from learnopengl.com, thus adhere to its original.

There are code from Learnopengl.com exists in this repository, and it is licensed as CC BY-NC 4.0 license
as published by Creative Commons, either version 4 of the License, or (at your option) any later version
(as can be seen [here](https://github.com/JoeyDeVries/LearnOpenGL/blob/master/LICENSE.md)).

Adapted or modified source code also adhere to such license as well, unless such code is totally new
and developed from start which you can use your own judgement comparing source code from learngopengl.com
and here.
