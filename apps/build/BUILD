# Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#load(":toolchain.bzl", "mke2fs_toolchain")
#
#toolchain_type(
#    name = "mke2fs_toolchain_type",
#)
#
#mke2fs_toolchain(
#    name = "mke2fs_toolchain",
#    binary = ":mke2fs",
#)
#
#toolchain(
#    name = "mke2fs_linux_toolchain",
#    exec_compatible_with = [
#        "@platforms//os:linux",
#    ],
#    toolchain = ":mke2fs_toolchain",
#    toolchain_type = ":mke2fs_toolchain_type",
#)

filegroup(
    name = "lds",
    srcs = ["apps.lds"],
    visibility = ["//apps:__subpackages__"],
)
