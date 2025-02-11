#include <stdio.h>
#include <kernel/tests/pagetest.h>


// Paging Test Function
void paging_test() {
    printf("\n[TEST] Running Paging Test...\n");

    // Step 1: Map a virtual address
    uint32_t virtual_addr = 0xC0100000; // Arbitrary high virtual address
    uint32_t physical_addr = 0x00200000; // Map to 2MB in physical memory

    printf("[TEST] Mapping Virtual Address 0x%x to Physical Address 0x%x\n",
           virtual_addr, physical_addr);
    vmm_map(virtual_addr, physical_addr, 1, 1); // User, RW

    // Step 2: Write and Read from Virtual Address
    uint32_t* test_page = (uint32_t*)virtual_addr;
    *test_page = 0xDEADBEEF; // Write test value
    printf("[TEST] Wrote 0x%x to 0x%x\n", *test_page, virtual_addr);

    // Step 3: Verify Mapping
    if (*test_page == 0xDEADBEEF) {
        printf("[TEST] Successfully verified mapped memory.\n");
    } else {
        printf("[TEST] Memory verification failed! Read 0x%x instead.\n", *test_page);
    }

    // Step 4: Trigger a Page Fault (Access Unmapped Memory)
    printf("[TEST] Attempting to access unmapped memory...\n");
    uint32_t* invalid_address = (uint32_t*)0xD0000000; // Unmapped region
    printf("Value at 0x%x: 0x%x\n", invalid_address, *invalid_address); // This should cause a page fault

    printf("[TEST] Paging Test Completed.\n");
}
