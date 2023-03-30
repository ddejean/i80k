# Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

# Target toolchain
TARGET_PREFIX := build/gcc-ia16/bin/ia16-elf-
TARGET_CC := $(TARGET_PREFIX)gcc
TARGET_AS := $(TARGET_PREFIX)as
TARGET_AR := $(TARGET_PREFIX)ar
TARGET_LD := $(TARGET_PREFIX)ld

TARGET_CLFAGS := -Wall -Werror -march=i8088 -mtune=i8088 -mcmodel=tiny -nostdinc -isystem libc/include
TARGET_LDFLAGS := -nostdlib --oformat=binary -L build/gcc-ia16/lib/gcc/ia16-elf/6.3.0
TARGET_LDLIBS := -lgcc
TARGET_ASFLAGS :=

# Host toolchain
HOST_CC := clang
HOST_LD := clang
