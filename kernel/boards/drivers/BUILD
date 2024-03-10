# Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

cc_library(
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
        "//libc:c",
    ],
)
