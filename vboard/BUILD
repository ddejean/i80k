load("@rules_go//go:def.bzl", "go_binary")

go_binary(
    name = "vboard",
    srcs = [
        "io.go",
        "vboard.go",
    ],
    deps = [
        "//vboard/devices",
        "//vboard/kvm",
        "@org_golang_x_sys//unix",
    ],
)
