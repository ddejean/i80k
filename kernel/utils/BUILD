# Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

cc_library(
    name = "utils",
    srcs = [
        "ringbuffer.c",
        "ringbuffer.h",
    ],
    hdrs = [
        "ringbuffer.h",
    ],
    visibility = ["//visibility:public"],
    deps = select({
        "@platforms//os:none": ["//libc:c"],
        "@platforms//os:linux": [],
    }),
)

cc_test(
    name = "utils_test",
    size = "small",
    srcs = ["ringbuffer_test.cc"],
    deps = [
        ":utils",
        "@googletest//:gtest_main",
    ],
)
