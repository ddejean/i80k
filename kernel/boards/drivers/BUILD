# Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

load("//kernel/build:rules.bzl", "i80k_library")

i80k_library(
    name = "p8254",
    srcs = [
        "p8254.c",
        "p8254.h",
    ],
    hdrs = ["p8254.h"],
    includes = ["."],
    visibility = ["//kernel/boards:__subpackages__"],
    deps = [
        "//kernel/arch",
        "//kernel/boards:libboard",
    ],
)

i80k_library(
    name = "p8259a",
    srcs = [
        "p8259a.c",
        "p8259a.h",
    ],
    hdrs = ["p8259a.h"],
    includes = ["."],
    visibility = ["//kernel/boards:__subpackages__"],
    deps = [
        "//kernel/arch",
        "//kernel/boards:libboard",
    ],
)
