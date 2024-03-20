# Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

cc_library(
    name = "ext2",
    srcs = glob([
        "*.c",
        "*.h",
    ]),
    visibility = ["//visibility:public"],
    deps = [
        "//kernel/core",
        "//libc:c",
    ],
    alwayslink = True,
)
