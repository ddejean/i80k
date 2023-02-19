// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include "tests_hwalloc.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <assert.h>

#include <CUnit/CUnit.h>

#include "hwalloc.h"

#define SEG_FIRST   1
#define SEG_LAST    2

int tests_hwalloc_init(void) {
    hw_alloc_init(SEG_FIRST, SEG_LAST);
    return 0;
}

void test_hwalloc_all(void) {
    struct hw_page *pages[128];

    memset(pages, 0, sizeof(pages));
    for (int i = 0; i < 128; i++) {
        pages[i] = hw_page_alloc();
        CU_ASSERT_PTR_NOT_NULL_FATAL(pages[i]);
        uint16_t seg = hw_page_seg(pages[i]);
        if (i < 64) {
            CU_ASSERT_EQUAL_FATAL(seg, 0x1000);
        } else {
            CU_ASSERT_EQUAL_FATAL(seg, 0x2000);
        }
    }
    for (int i = 0; i < 128; i++) {
        hw_page_free(pages[i]);
    }

}

void test_hwalloc_alloc_and_free(void) {
    struct hw_page *p1, *p2;

    p1 = hw_page_alloc();
    CU_ASSERT_PTR_NOT_NULL_FATAL(p1);
    CU_ASSERT_EQUAL_FATAL(hw_page_seg(p1), 0x1000);
    CU_ASSERT_EQUAL_FATAL(hw_page_addr(p1), (uint16_t*)0x0000);

    p2 = hw_page_alloc();
    CU_ASSERT_PTR_NOT_NULL_FATAL(p2);
    CU_ASSERT_EQUAL_FATAL(hw_page_seg(p2), 0x1000);
    CU_ASSERT_EQUAL_FATAL(hw_page_addr(p2), (uint16_t*)0x400);

    hw_page_free(p1);

    p1 = hw_page_alloc();
    CU_ASSERT_PTR_NOT_NULL_FATAL(p1);
    CU_ASSERT_EQUAL_FATAL(hw_page_seg(p1), 0x1000);
    CU_ASSERT_EQUAL_FATAL(hw_page_addr(p1), (uint16_t*)0x0000);

    hw_page_free(p1);
    hw_page_free(p2);
}
