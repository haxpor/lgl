#!/bin/bash

declare -A DEMOS
declare -a DEMOSOR
DEMOS["Barebone.cpp"]="Bare minimal just show window with no rendering";
DEMOSOR+=("Barebone.cpp")

DEMOS["HelloTriangle.cpp"]="Basic triangle primitive rendering";
DEMOSOR+=("HelloTriangle.cpp")

DEMOS["EBO.cpp"]="Element Buffer Object (indexed rendering)";
DEMOSOR+=("EBO.cpp")

DEMOS["GLQuery.cpp"]="Simple OpenGL information query";
DEMOSOR+=("GLQuery.cpp")

DEMOS["Shader.cpp"]="Shader with uniform";
DEMOSOR+=("Shader.cpp")

DEMOS["Shader2.cpp"]="Shader with more vertex attributes";
DEMOSOR+=("Shader2.cpp")

DEMOS["Shader2_with_FileReader.cpp"]="Based on Shader2.cpp but with FileReader";
DEMOSOR+=("Shader2_with_FileReader.cpp")

DEMOS["Textures.cpp"]="Texture mapping. Sampling texture color from texture";
DEMOSOR+=("Textures.cpp")

DEMOS["Transformations.cpp"]="Basic applying of transformations on 2 quads";
DEMOSOR+=("Transformations.cpp")

DEMOS["Transformations2.cpp"]="Rotating cubes";
DEMOSOR+=("Transformations2.cpp")

DEMOS["Camera.cpp"]="Camera setup and manipulation";
DEMOSOR+=("Camera.cpp")

print_help() {
    echo "Usage: ./build.sh <command> <optional-parameter>"
    echo "List of commands"
    printf "%10s\t-\t%s\n" "build" "Usage: build <.cpp source file>"
    printf "          \t \t%s\n"   "Build a demo program"
    printf "%10s\t-\t%s\n" "list"  "List all demo programs along with"
    printf "          \t \t%s\n"   "corresponding descriptions"
    printf "%10s\t-\t%s\n" "help"  "Show help text"
}

if [ "$#" -lt 1 ]; then
    print_help
    exit 1
fi

if [ "$1" == "build" ]; then
    if [ "$#" -lt 2 ]; then
        echo "Missing .cpp source file"
        echo "Usage: ./make.sh build <.cpp source file>"
        exit 1
    fi

    g++ -g -fno-exceptions -Wno-stringop-overflow -std=c++11 \
        -Iexternals/glad/include \
        -Iexternals/stb_image \
        -Iincludes \
        src/lgl/*.cpp \
        externals/glad/src/glad.c \
        $2 \
        -lglfw3 -lGL -lX11 -lpthread -lm -ldl

elif [ "$1" == "list" ]; then
    for i in "${!DEMOSOR[@]}"; do
        printf "%30s - %s\n" "${DEMOSOR[$i]}" "${DEMOS[${DEMOSOR[$i]}]}"
    done
    
else
    print_help
    exit 1
fi
