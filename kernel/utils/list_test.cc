// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include <gtest/gtest.h>
#include <string.h>

extern "C" {
#include "list.h"
}

struct elem {
    struct list_node node;
    int value;
};

class ListTest : public ::testing::Test {
   protected:
    void SetUp() override { list_initialize(&list); }

    struct list_node list;
};

TEST_F(ListTest, Empty) {
    EXPECT_TRUE(list_is_empty(&list));
    EXPECT_EQ(0, list_length(&list));
}

TEST_F(ListTest, SimpleList) {
    struct elem elems[16];

    // Create a 16 elements list.
    for (int i = 0; i < 16; i++) {
        elems[i].value = i;
        list_add_tail(&list, &elems[i].node);
    }
    EXPECT_FALSE(list_is_empty(&list));
    EXPECT_EQ(16, list_length(&list));
}
