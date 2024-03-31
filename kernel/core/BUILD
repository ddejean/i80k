# Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

cc_library(
    name = "core",
    srcs = glob([
        "*.h",
        "*.c",
        "*.S",
    ]),
    hdrs = glob(["*.h"]),
    includes = ["."],
    visibility = ["//visibility:public"],
    deps = select({
        "@platforms//os:none": ["//libc:c"],
        "@platforms//os:linux": [],
    }),
)

cc_test(
    name = "core_test",
    size = "small",
    srcs = glob([
        "tests/*.cc",
        "tests/*.h",
    ]),
    # Required for strlcpy.
    linkopts = ["-lbsd"],
    deps = [
        ":core",
        "@googletest//:gtest_main",
    ],
)
