#!/bin/sh

clang-format-4.0 \
    -style=file \
    -i \
    parseconf.c \
    parseconf.h \
    example/example.c
