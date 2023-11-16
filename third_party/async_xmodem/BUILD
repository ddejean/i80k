cc_library(
    name = "async_xmodem",
    srcs = ["xmodem_server.c"],
    hdrs = ["xmodem_server.h"],
    includes = ["."],
    visibility = ["//visibility:public"],
    deps = select({
        "@platforms//os:none": ["//libc:c"],
        "@platforms//os:linux": [],
    }),
)

cc_test(
    name = "async_xmodem_test",
    size = "small",
    srcs = [
        "acutest.h",
        "xmodem_server_test.c",
    ],
    deps = [
        ":async_xmodem",
    ],
)
