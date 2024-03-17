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

void normalizePath(const char *in, char *out, size_t sz) {
    strncpy(out, in, sz - 1);
    out[sz - 1] = '\0';
    fs_normalize_path(out);
}

TEST_F(FsTest, NormalizePath) {
    char path[FS_MAX_PATH_LEN];

    normalizePath("/", path, sizeof(path));
    EXPECT_STREQ("", path);
    normalizePath("/test", path, sizeof(path));
    EXPECT_STREQ("/test", path);
    normalizePath("/test/", path, sizeof(path));
    EXPECT_STREQ("/test", path);
    normalizePath("test/", path, sizeof(path));
    EXPECT_STREQ("test", path);
    normalizePath("test", path, sizeof(path));
    EXPECT_STREQ("test", path);
    normalizePath("/test//", path, sizeof(path));
    EXPECT_STREQ("/test", path);
    normalizePath("/test/foo", path, sizeof(path));
    EXPECT_STREQ("/test/foo", path);
    normalizePath("/test/foo/", path, sizeof(path));
    EXPECT_STREQ("/test/foo", path);
    normalizePath("/test/foo/bar", path, sizeof(path));
    EXPECT_STREQ("/test/foo/bar", path);
    normalizePath("/test/foo/bar//", path, sizeof(path));
    EXPECT_STREQ("/test/foo/bar", path);
    normalizePath("/test//foo/bar//", path, sizeof(path));
    EXPECT_STREQ("/test/foo/bar", path);
    normalizePath("/test//./foo/bar//", path, sizeof(path));
    EXPECT_STREQ("/test/foo/bar", path);
    normalizePath("/test//./.foo/bar//", path, sizeof(path));
    EXPECT_STREQ("/test/.foo/bar", path);
    normalizePath("/test//./..foo/bar//", path, sizeof(path));
    EXPECT_STREQ("/test/..foo/bar", path);
    normalizePath("/test//./../foo/bar//", path, sizeof(path));
    EXPECT_STREQ("/foo/bar", path);
    normalizePath("/test/../foo", path, sizeof(path));
    EXPECT_STREQ("/foo", path);
    normalizePath("/test/bar/../foo", path, sizeof(path));
    EXPECT_STREQ("/test/foo", path);
    normalizePath("../foo", path, sizeof(path));
    EXPECT_STREQ("foo", path);
    normalizePath("../foo/", path, sizeof(path));
    EXPECT_STREQ("foo", path);
    normalizePath("/../foo", path, sizeof(path));
    EXPECT_STREQ("foo", path);
    normalizePath("/../foo/", path, sizeof(path));
    EXPECT_STREQ("foo", path);
    normalizePath("/../../foo", path, sizeof(path));
    EXPECT_STREQ("foo", path);
    normalizePath("/bleh/../../foo", path, sizeof(path));
    EXPECT_STREQ("foo", path);
    normalizePath("/bleh/bar/../../foo", path, sizeof(path));
    EXPECT_STREQ("/foo", path);
    normalizePath("/bleh/bar/../../foo/..", path, sizeof(path));
    EXPECT_STREQ("", path);
    normalizePath("/bleh/bar/../../foo/../meh", path, sizeof(path));
    EXPECT_STREQ("/meh", path);
}

TEST_F(FsTest, Mount) {
    EXPECT_EQ(-1, fs_mount("", "testfs", "dev0"));
    EXPECT_EQ(-1, fs_mount("/", "testfs", ""));
    EXPECT_EQ(-1, fs_mount("/", "testfs", "dev0"));
}