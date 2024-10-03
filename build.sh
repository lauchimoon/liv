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
    cd raylib/src/
    # Modify config to allow all filetypes
    sed 's/\/\/#define SUPPORT_FILEFORMAT/#define SUPPORT_FILEFORMAT/' config.h > tmp.h; mv tmp.h config.h
    make
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

DEBUG_FLAG='-DLIV_DEBUG'

existing=$(check_existing_dirs)
if [[ $existing = "false" ]]; then
    setup_raylib
fi

if [[ $1 = 'debug' ]]; then
    $CC $SRC $LDLIBS $INCLUDE $DEBUG_FLAG -o $OUT
else
    $CC $SRC $LDLIBS $INCLUDE -o $OUT
fi
