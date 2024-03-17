// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _FAKE_DEV_H_
#define _FAKE_DEV_H_

#include <stddef.h>
#include <stdint.h>

#define TEST_BLOCK_SZ 16
#define TEST_BLOCK_CNT 4
#define TEST_FULL_SZ (TEST_BLOCK_CNT * TEST_BLOCK_SZ)

class FakeDev {
   public:
    FakeDev();
    int BlockRead(void *buf, uint32_t block, size_t count);
    int BlockWrite(const void *buf, uint32_t block, size_t count);
    bool has_block_read() const { return block_read; }
    bool has_block_write() const { return block_write; }

   private:
    bool block_read;
    bool block_write;
    uint8_t buffer[TEST_FULL_SZ];
};

#endif  // _FAKE_DEV_H_