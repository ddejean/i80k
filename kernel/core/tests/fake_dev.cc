// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#include "fake_dev.h"

#include <string.h>

FakeDev::FakeDev() : block_read(false), block_write(false) {
    for (int i = 0; i < TEST_FULL_SZ; i++) {
        this->buffer[i] = (uint8_t)i;
    }
}

int FakeDev::BlockRead(void *buf, uint32_t block, size_t count) {
    block_read = true;
    memcpy(buf, &(this->buffer[block * TEST_BLOCK_SZ]), count * TEST_BLOCK_SZ);
    return count * TEST_BLOCK_SZ;
}

int FakeDev::BlockWrite(const void *buf, uint32_t block, size_t count) {
    block_write = true;
    memcpy(&(this->buffer[block * TEST_BLOCK_SZ]), buf, count * TEST_BLOCK_SZ);
    return count * TEST_BLOCK_SZ;
}
