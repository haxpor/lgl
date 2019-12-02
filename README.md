# Build

Most examples (preferred "testbed") are self-contained and have their own separate `Makefile` in its directory,
link against few shared source code in `src/../externals`.

Others, mostly inside `Misc` and
other which only has main source file can be built via `make.sh` script.

## Build via `make.sh`

At the root directory level of this repo, Execute `./make.sh` like the following

`./make.sh build <target .cpp file>`

for example

`./make.sh build src/Shader.cpp`

it will produce `a.out` file at the root directory, run it with `./a.out`. Any resource it needs to
access is inside `data` directory.

It supprots following commands

* `./make.sh build src/<directory>/<main-source-file.cpp>` - to build target source file, provided that main function is in this file
* `./make.sh help` - print help message

# Dependencies

Included in `external/` directory

* `glad` - generated from its webservice for OpenGL 3.3 API, Core profile (subject to be gradually added more API support later if need)
* `stb_image` - for loading image asset

Others

* `glfw` - for window stuff
* `glm` - for OpenGL Mathematics

# Note

* At the near end of completion of study (thus this project), `CMake` build script will be added for convenience in building all demo programs. For now, it makes no sense to incur having it while studying. Modify `make.sh` as needed for your system to make it work for the time being.
* **Beware**: Tested only on Linux, technically should work the same on other platforms but might need slight modification on build script, or particular code. Also some sample make uses of symlink to get access to `../../data` conveniently.
* Code is aimed to be as educational for studying purpose as first priority, so not all optimization detail will be handled, nor fully clean code will be architected.

# License

All non-source asset from learnopengl.com, thus adhere to its original.

There are code from Learnopengl.com exists in this repository, and it is licensed as CC BY-NC 4.0 license
as published by Creative Commons, either version 4 of the License, or (at your option) any later version
(as can be seen [here](https://github.com/JoeyDeVries/LearnOpenGL/blob/master/LICENSE.md)).

Adapted or modified source code also adhere to such license as well, unless such code is totally new
and developed from start which you can use your own judgement comparing source code from learngopengl.com
and here.
