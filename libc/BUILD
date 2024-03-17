# Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

cc_library(
    name = "c",
    srcs = glob([
        "*.h",
        "*.c",
        "*.S",
        "string/*.c",
        "string/*.S",
    ]),
    hdrs = glob([
        "*.h",
        "include/*.h",
        "include/bsd/*.h",
    ]),
    includes = ["include"],
    visibility = ["//visibility:public"],
)
