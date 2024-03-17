// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#include <gtest/gtest.h>

extern "C" {
#include "blkdev.h"
#include "fs.h"
}

extern "C" const struct fs _fs_start[1] = {{
    .name = "testfs",
    .api = NULL,
}};
extern "C" const struct fs _fs_end[1] = {};

class FsTest : public ::testing::Test {
   public:
    void SetUp() override {}

    void TearDown() override {}
};

TEST_F(FsTest, TrimName) {
    EXPECT_STREQ("path", trim_name("/path"));
    EXPECT_STREQ("path", trim_name(" path"));
    EXPECT_STREQ("path", trim_name(" /path"));
    EXPECT_STREQ("path", trim_name("  //path"));
}

TEST_F(FsTest, NormalizePath) {
    char path[FS_MAX_PATH_LEN];

    strncpy(path, "/folder/file", FS_MAX_PATH_LEN - 1);
    fs_normalize_path(path);
    EXPECT_STREQ("/folder/file", path);

    strncpy(path, "/folder/../file", FS_MAX_PATH_LEN - 1);
    fs_normalize_path(path);
    EXPECT_STREQ("/file", path);

    strncpy(path, "/path//to/a/../file", FS_MAX_PATH_LEN - 1);
    fs_normalize_path(path);
    EXPECT_STREQ("/path/to/file", path);

    strncpy(path, "/a/folder/", FS_MAX_PATH_LEN - 1);
    fs_normalize_path(path);
    EXPECT_STREQ("/a/folder", path);
}

TEST_F(FsTest, Mount) {
    EXPECT_EQ(-1, fs_mount("", "testfs", "dev0"));
    EXPECT_EQ(-1, fs_mount("/", "testfs", ""));
    EXPECT_EQ(-1, fs_mount("/", "testfs", "dev0"));
}