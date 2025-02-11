#include <stdio.h>
#include <kernel/tests/pagetest.h>

// Paging Test Function
void paging_test() {
    printf("\n[TEST] Running Paging Test...\n");

    // Use a virtual address in the identity mapped region (e.g., 0x00300000)
    uint32_t virtual_addr = 0x00300000;
    uint32_t physical_addr = 0x00200000;

    printf("[TEST] Mapping Virtual Address 0x%x to Physical Address 0x%x\n",
           virtual_addr, physical_addr);
    vmm_map(virtual_addr, physical_addr, 1, 1); // Create the mapping

    uint32_t* test_page = (uint32_t*)virtual_addr;
    *test_page = 0xDEADBEEF; // Write a test value

    printf("[TEST] Read 0x%x from 0x%x\n", *test_page, virtual_addr);

    if (*test_page == 0xDEADBEEF) {
        printf("[TEST] Successfully verified mapped memory.\n");
    } else {
        printf("[TEST] Memory verification failed! Read 0x%x instead.\n", *test_page);
    }
}
