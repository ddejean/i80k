# Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

load("//apps/build:rules.bzl", "app_binary")
load("//apps/build:image.bzl", "ext2_image")

app_binary(
    name = "test",
    srcs = ["main.c"],
)

ext2_image(
    name = "fs.img",
    block_size = 1024,
    # 64 MB or 61MiB
    fs_size = 62592,
    options = ["^large_file"],
    deps = [
        ":test",
    ],
)
