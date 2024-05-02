# Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

load("//kernel/build:rules.bzl", "i80k_library")

i80k_library(
    name = "arch",
    srcs = glob([
        "*.h",
        "*.c",
        "*.S",
    ]),
    hdrs = glob(["*.h"]),
    includes = ["."],
    visibility = ["//visibility:public"],
)
