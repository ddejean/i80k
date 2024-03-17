# Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

cc_library(
    name = "c",
    srcs = glob([
        "*.c",
        "*.S",
        "string/*.c",
        "string/*.S",
    ]),
    hdrs = glob([
        "*.h",
        "include/*.h",
        "include/bsd/*.h",
        "include/sys/*.h",
    ]),
    includes = ["include"],
    visibility = ["//visibility:public"],
)
