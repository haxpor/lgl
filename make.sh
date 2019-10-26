#!/bin/bash

declare -A DEMOS
declare -a DEMOSOR

print_help() {
    echo "Usage: ./make.sh <command> <optional-parameter>"
    echo "List of commands"
    printf "%10s\t-\t%s\n" "build" "Usage: build <.cpp source file>"
    printf "          \t \t%s\n"   "Build a demo program"
    printf "%10s\t-\t%s\n" "help"  "Show help text"
}

if [ "$#" -lt 1 ]; then
    print_help
    exit 1
fi

if [ "$1" == "build" ]; then
    if [ "$#" -lt 2 ]; then
        echo "Missing .cpp source file"
        echo "Usage: ./make.sh buildsc <.cpp source file>"
        exit 1
    fi

    g++ -g -Wall -Wextra -pedantic -fno-exceptions -Wno-stringop-overflow -Wno-unused-parameter -std=c++11 \
        -Iexternals/glad/include \
        -Iexternals/stb_image \
        -Iincludes \
        src/lgl/*.cpp \
        externals/glad/src/glad.c \
        $2 \
        -lglfw -lGL -lX11 -lpthread -lm -ldl

elif [ "$1" == "help" ]; then
    print_help
    exit 1
    
else
    print_help
    exit 1
fi
