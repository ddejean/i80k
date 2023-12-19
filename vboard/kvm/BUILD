load("@rules_go//go:def.bzl", "go_library")

go_library(
    name = "kvm",
    srcs = glob([
        "*.go",
    ]),
    importpath = "vboard/kvm",
    visibility = ["//visibility:public"],
    deps = [
        "@org_golang_x_sys//unix",
    ],
)
