// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include <gtest/gtest.h>
#include <string.h>

extern "C" {
#include "ringbuffer.h"
}

class RingBufferTest : public ::testing::Test {
   protected:
    void SetUp() override {
        memset(buf, 0, sizeof(buf));
        ring_buffer_init(&rb, buf, sizeof(buf));
    }

    long unsigned rb_size() const { return sizeof(buf) - 1; }

    // Buffer that will back the ring buffer implementation.
    char buf[16];
    // Ring buffer under test.
    struct ring_buffer_t rb;
};

// Demonstrate some basic assertions.
TEST_F(RingBufferTest, EmptyOrFull) {
    EXPECT_EQ(1, ring_buffer_is_empty(&rb));

    // Fill the buffer and check it's full.
    for (long unsigned i = 0; i < rb_size(); i++) {
        ring_buffer_queue(&rb, 'a');
    }
    EXPECT_EQ(1, ring_buffer_is_full(&rb));
    EXPECT_EQ(sizeof(buf) - 1, ring_buffer_num_items(&rb));

    // Empty the buffer and check values.
    for (long unsigned i = 0; i < rb_size(); i++) {
        char c = 0;
        EXPECT_EQ(1, ring_buffer_dequeue(&rb, &c));
        EXPECT_EQ('a', c);
    }
    EXPECT_EQ(1, ring_buffer_is_empty(&rb));
}

TEST_F(RingBufferTest, SingleReadAndWrite) {
    ring_buffer_queue(&rb, 'a');
    ring_buffer_queue(&rb, 'b');
    ring_buffer_queue(&rb, 'c');
    ring_buffer_queue(&rb, 'd');
    EXPECT_EQ(0, ring_buffer_is_full(&rb));

    char c = 0;
    EXPECT_EQ(1, ring_buffer_dequeue(&rb, &c));
    EXPECT_EQ('a', c);
    EXPECT_EQ(1, ring_buffer_dequeue(&rb, &c));
    EXPECT_EQ('b', c);
    EXPECT_EQ(1, ring_buffer_dequeue(&rb, &c));
    EXPECT_EQ('c', c);
    EXPECT_EQ(1, ring_buffer_dequeue(&rb, &c));
    EXPECT_EQ('d', c);
}

TEST_F(RingBufferTest, ArrayReadAndWrite) {
    const char *input = "abcd";
    ring_buffer_queue_arr(&rb, input, strlen(input));
    EXPECT_EQ(0, ring_buffer_is_full(&rb));

    char result[4];
    EXPECT_EQ(sizeof(result),
              ring_buffer_dequeue_arr(&rb, result, sizeof(result)));
    EXPECT_EQ(0, strncmp(input, result, sizeof(result)));
    EXPECT_EQ(1, ring_buffer_is_empty(&rb));
}

TEST_F(RingBufferTest, WriteArrayWithOverflow) {
    // Write a string bigger than the buffer.
    const char *input = "abcdefghijklmnopqrst";
    ring_buffer_queue_arr(&rb, input, strlen(input));
    EXPECT_EQ(1, ring_buffer_is_full(&rb));

    // Check that all the last characters of the string are in the buffer.
    char result[16];
    EXPECT_EQ(rb_size(), ring_buffer_dequeue_arr(&rb, result, sizeof(result)));
    EXPECT_EQ(0, strncmp("fghijklmnopqrst", result, rb_size()));
    EXPECT_EQ(1, ring_buffer_is_empty(&rb));
}