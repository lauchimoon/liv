#!/usr/bin/bash
function check_existing_dirs() {
    if [[ ! -d "lib/" || ! -d "include/" ]]; then
        echo "false"
        exit 0
    fi

    echo "true"
}

function setup_raylib() {
    mkdir include lib
    git clone --depth=1 "https://github.com/raysan5/raylib.git"
    cd raylib/src/ && make
    cd ../../
    cp raylib/src/libraylib.a lib/
    cp raylib/src/raylib.h include/
    yes | rm -r raylib/
}

CC=gcc
SRC=liv.c
LDLIBS='-L./lib/ -lraylib -lm -ldl -lpthread -lGL'
INCLUDE='-I./include/'
OUT=liv

existing=$(check_existing_dirs)
if [[ $existing = "false" ]]; then
    setup_raylib
fi

$CC $SRC $LDLIBS $INCLUDE -o $OUT
