#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Usage: ./build.sh <main-source-file>"
    exit 1
fi

g++ -g -fno-exceptions -std=c++11 \
    -I../external/glad/include \
    -I../external/stb_image \
    -Icore \
    core/*.cpp \
    ../external/glad/src/glad.c \
    $1 \
    -lglfw3 -lGL -lX11 -lpthread -ldl
