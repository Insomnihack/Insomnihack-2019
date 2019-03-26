#!/usr/bin/env bash
gcc -Wall -I/usr/local/include \
    -o boogy.so \
    boogy.c \
    -shared \
    -Wno-implicit-function-declaration \
    -Wno-unused-variable \
    -Wno-unused-but-set-variable \
    -Wno-int-conversion \
    -Wno-parentheses \
    -fPIC \
    -fno-stack-protector \
    -fomit-frame-pointer
strip boogy.so
