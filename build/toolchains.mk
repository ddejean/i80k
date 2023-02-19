# Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

# Target toolchain
TARGET_CC := bcc
TARGET_AS := nasm
TARGET_AR := ar86
TARGET_LD := ld86

TARGET_CLFAGS := -0 -ansi -I
TARGET_ASFLAGS := -f as86

# Host toolchain
HOST_CC := clang
HOST_LD := clang
