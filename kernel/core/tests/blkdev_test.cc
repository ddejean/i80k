// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#include <gtest/gtest.h>

#include "fake_dev.h"

extern "C" {
#include "blkdev.h"
#include "error.h"
}

// Test device names.
char dev0_name[] = "dev0";
char dev1_name[] = "dev1";

extern "C" int testReadBlock(const struct blkdev *dev, void *buf,
                             uint32_t block, size_t count) {
    FakeDev *fd = reinterpret_cast<FakeDev *>(dev->drv_data);
    return fd->BlockRead(buf, block, count);
}

extern "C" int testWriteBlock(const struct blkdev *dev, const void *buf,
                              uint32_t block, size_t count) {
    FakeDev *fd = reinterpret_cast<FakeDev *>(dev->drv_data);
    return fd->BlockWrite(buf, block, count);
}

class BlkdevTest : public ::testing::Test {
   public:
    void SetUp() override {
        dev = (struct blkdev *)malloc(sizeof(*dev));
        dev->name = dev0_name;
        dev->block_count = TEST_BLOCK_CNT;
        dev->block_size = TEST_BLOCK_SZ;
        dev->block_shift = 4;
        dev->drv_data = new FakeDev();
        dev->read_block = testReadBlock;
        dev->write_block = testWriteBlock;
        blk_register(dev);
    }

    void TearDown() override {
        struct blkdev *tmp;
        tmp = blk_unregister(dev0_name);
        if (tmp) {
            FakeDev *fd = static_cast<FakeDev *>(tmp->drv_data);
            delete fd;
            free(tmp);
        }
    }

    const struct blkdev *device() { return dev; }

    bool has_block_read() {
        FakeDev *fd = static_cast<FakeDev *>(dev->drv_data);
        return fd->has_block_read();
    }

    bool has_block_write() {
        FakeDev *fd = static_cast<FakeDev *>(dev->drv_data);
        return fd->has_block_write();
    }

   private:
    struct blkdev *dev;
};

TEST_F(BlkdevTest, RegisterUnregister) {
    struct blkdev *dev = (struct blkdev *)malloc(sizeof(*dev));
    dev->name = dev1_name;

    EXPECT_EQ(NULL, blk_open(dev1_name));
    blk_register(dev);
    EXPECT_EQ(dev, blk_open(dev1_name));
    blk_unregister(dev1_name);
    EXPECT_EQ(NULL, blk_open(dev1_name));

    dev->name = NULL;
    free(dev);
}

TEST_F(BlkdevTest, Open) {
    EXPECT_EQ(NULL, blk_open(NULL));
    EXPECT_EQ(NULL, blk_open(dev1_name));
    EXPECT_EQ(device(), blk_open("dev0"));
}

TEST_F(BlkdevTest, BlockTrimRange) {
    EXPECT_EQ(0, blk_block_trim_range(device(), 5, 1));
    EXPECT_EQ(0, blk_block_trim_range(device(), 3, 0));
    EXPECT_EQ(1, blk_block_trim_range(device(), 3, 2));
    EXPECT_EQ(1, blk_block_trim_range(device(), 0, 1));
    EXPECT_EQ(4, blk_block_trim_range(device(), 0, 4));
    EXPECT_EQ(2, blk_block_trim_range(device(), 1, 2));
}

TEST_F(BlkdevTest, NoBlockRead) {
    char buf[32];
    EXPECT_EQ(ERR_INVAL, blk_read_block(NULL, buf, 0, 1));
    EXPECT_FALSE(has_block_read());
    EXPECT_EQ(ERR_INVAL, blk_read_block(device(), NULL, 0, 1));
    EXPECT_FALSE(has_block_read());
    EXPECT_EQ(0, blk_read_block(device(), buf, 5, 1));
    EXPECT_FALSE(has_block_read());
}

TEST_F(BlkdevTest, BlockReadOneBlock) {
    char buf[TEST_BLOCK_SZ];

    EXPECT_EQ(TEST_BLOCK_SZ, blk_read_block(device(), buf, 0, 1));
    EXPECT_TRUE(has_block_read());
    for (int i = 0; i < TEST_BLOCK_SZ; i++) {
        EXPECT_EQ(i, buf[i]);
    }
}

TEST_F(BlkdevTest, BlockReadTwoBlocksWithOffset) {
    char buf[2 * TEST_BLOCK_SZ];

    EXPECT_EQ(2 * TEST_BLOCK_SZ, blk_read_block(device(), buf, 1, 2));
    EXPECT_TRUE(has_block_read());
    for (int i = 0; i < 2 * TEST_BLOCK_SZ; i++) {
        EXPECT_EQ(TEST_BLOCK_SZ + i, buf[i]);
    }
}

TEST_F(BlkdevTest, BlockReadFull) {
    char buf[TEST_FULL_SZ];

    EXPECT_EQ(TEST_FULL_SZ, blk_read_block(device(), buf, 0, TEST_BLOCK_CNT));
    EXPECT_TRUE(has_block_read());

    for (int i = 0; i < TEST_FULL_SZ; i++) {
        EXPECT_EQ((char)i, buf[i]);
    }
}

TEST_F(BlkdevTest, ReadPartial) {
    char buf[30];

    memset(buf, 0, 30);
    EXPECT_EQ(30, blk_read(device(), buf, 10, 30));
    for (int i = 0; i < 30; i++) {
        EXPECT_EQ(i + 10, buf[i]);
    }
}

TEST_F(BlkdevTest, ReadFull) {
    char buf[TEST_FULL_SZ];

    memset(buf, 0, TEST_FULL_SZ);
    EXPECT_EQ(TEST_FULL_SZ, blk_read(device(), buf, 0, TEST_FULL_SZ));
    for (int i = 0; i < TEST_FULL_SZ; i++) {
        EXPECT_EQ(i, buf[i]);
    }
}

TEST_F(BlkdevTest, NoBlockWrite) {
    char buf[32];

    memset(buf, 0, 32);
    EXPECT_EQ(ERR_INVAL, blk_write_block(NULL, buf, 0, 1));
    EXPECT_FALSE(has_block_write());
    EXPECT_EQ(ERR_INVAL, blk_write_block(device(), NULL, 0, 1));
    EXPECT_FALSE(has_block_write());
    EXPECT_EQ(0, blk_write_block(device(), buf, 5, 1));
    EXPECT_FALSE(has_block_write());
}

TEST_F(BlkdevTest, BlockWriteOneBlock) {
    char buf[TEST_BLOCK_SZ];

    memset(buf, 0, TEST_BLOCK_SZ);
    EXPECT_EQ(TEST_BLOCK_SZ, blk_write_block(device(), buf, 0, 1));
    EXPECT_TRUE(has_block_write());
}

TEST_F(BlkdevTest, BlockWriteTwoBlocksWithOffset) {
    char buf[2 * TEST_BLOCK_SZ];

    memset(buf, 0, 2 * TEST_BLOCK_SZ);
    EXPECT_EQ(2 * TEST_BLOCK_SZ, blk_write_block(device(), buf, 1, 2));
    EXPECT_TRUE(has_block_write());

    memset(buf, 0xff, 2 * TEST_BLOCK_SZ);
    EXPECT_EQ(2 * TEST_BLOCK_SZ, blk_read_block(device(), buf, 1, 2));
    for (int i = 0; i < 2 * TEST_BLOCK_SZ; i++) {
        EXPECT_EQ(0, buf[i]);
    }
}

TEST_F(BlkdevTest, BlockWriteFull) {
    char buf[TEST_FULL_SZ];

    memset(buf, 0, TEST_FULL_SZ);
    EXPECT_EQ(TEST_FULL_SZ, blk_write_block(device(), buf, 0, TEST_BLOCK_CNT));
    EXPECT_TRUE(has_block_write());

    memset(buf, 0xff, TEST_FULL_SZ);
    EXPECT_EQ(TEST_FULL_SZ, blk_read_block(device(), buf, 0, TEST_BLOCK_CNT));
    for (int i = 0; i < TEST_FULL_SZ; i++) {
        EXPECT_EQ(0, buf[i]);
    }
}

TEST_F(BlkdevTest, WritePartial) {
    char buf[23];

    memset(buf, 0, 23);
    EXPECT_EQ(23, blk_write(device(), buf, 12, 23));
    memset(buf, 0xff, 23);
    EXPECT_EQ(23, blk_read(device(), buf, 12, 23));
    for (int i = 0; i < 23; i++) {
        EXPECT_EQ(0, buf[i]);
    }
}

TEST_F(BlkdevTest, WriteFull) {
    char buf[TEST_FULL_SZ];

    memset(buf, 0, TEST_FULL_SZ);
    EXPECT_EQ(TEST_FULL_SZ, blk_write(device(), buf, 0, TEST_FULL_SZ));
    memset(buf, 0xff, TEST_FULL_SZ);
    EXPECT_EQ(TEST_FULL_SZ, blk_read(device(), buf, 0, TEST_FULL_SZ));
    for (int i = 0; i < TEST_FULL_SZ; i++) {
        EXPECT_EQ(0, buf[i]);
    }
}
