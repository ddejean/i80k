// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

#include "tests_hwalloc.h"

int main(int argc, char **argv) {
    CU_pSuite tests;

    CU_initialize_registry();
    tests = CU_add_suite("hwalloc", tests_hwalloc_init, NULL);
    CU_ADD_TEST(tests, test_hwalloc_all);
    CU_ADD_TEST(tests, test_hwalloc_alloc_and_free);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return 0;
}