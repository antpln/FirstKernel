#ifndef KERNEL_MEMTEST_H
#define KERNEL_MEMTEST_H

#include <stdint.h>

class MemoryTester {
public:
    static bool test_allocation();
    static bool test_free();
    static bool test_multiple_allocations();
    static bool test_boundary_conditions();
private:
    static const uint32_t TEST_PATTERN = 0xAA55AA55;
};

#endif