# Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

cc_library(
    name = "utils",
    srcs = [
        "list.h",
        "ringbuffer.c",
        "ringbuffer.h",
    ],
    hdrs = [
        "list.h",
        "ringbuffer.h",
    ],
    includes = ["."],
    visibility = ["//visibility:public"],
    deps = select({
        "@platforms//os:none": ["//libc:c"],
        "@platforms//os:linux": [],
    }),
)

cc_test(
    name = "utils_test",
    size = "small",
    srcs = [
        "list_test.cc",
        "ringbuffer_test.cc",
    ],
    deps = [
        ":utils",
        "@googletest//:gtest_main",
    ],
)
