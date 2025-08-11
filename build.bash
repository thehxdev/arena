#!/usr/bin/env bash

set -xe

if [[ -z "$CC" ]]; then
    CC=cc
fi

CFLAGS=(-std=c99 -Wall -Wextra -Og -g3)

$CC "${CFLAGS[@]}" arena.c test.c
