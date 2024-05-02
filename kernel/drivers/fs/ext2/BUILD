# Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

load("//kernel/build:rules.bzl", "i80k_library")

i80k_library(
    name = "ext2",
    srcs = glob([
        "*.c",
        "*.h",
    ]),
    visibility = ["//visibility:public"],
    deps = [
        "//kernel/core",
    ],
    alwayslink = True,
)
