# Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

filegroup(
    name = "srcs",
    srcs = glob([
        "*.S",
        "*.h",
        "*.c",
    ]),
    visibility = ["//boards:__pkg__"],
)

filegroup(
    name = "hdrs",
    srcs = ["board.h"],
    visibility = ["//boards:__pkg__"],
)

filegroup(
    name = "lds",
    srcs = ["kernel.lds"],
    visibility = ["//boards:__pkg__"],
)
