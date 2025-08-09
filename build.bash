#!/usr/bin/env bash

set -xe

if [[ -z "$CC" ]]; then
    CC=cc
fi

CFLAGS=(-std=c89 -Wall -Wextra -Wpedantic -Og -g3)

$CC "${CFLAGS[@]}" arena.c main.c
