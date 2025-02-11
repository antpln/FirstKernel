#include <stdio.h>
#include <kernel/heap.h>
#include <kernel/tests/heaptest.h>

void heap_test() {
    printf("\n[TEST] Running Heap (kmalloc/kfree) Test...\n");

    // Allocate three blocks
    void* ptr1 = kmalloc(64);
    printf("[TEST] Allocated 64 bytes at %p\n", ptr1);

    void* ptr2 = kmalloc(128);
    printf("[TEST] Allocated 128 bytes at %p\n", ptr2);

    void* ptr3 = kmalloc(32);
    printf("[TEST] Allocated 32 bytes at %p\n", ptr3);

    // Check for overlapping allocations
    if (ptr1 && ptr2 && ptr3) {
        if (ptr1 < ptr2 && ptr2 < ptr3) {
            printf("[PASS] Allocations do not overlap and are correctly ordered.\n");
        } else {
            printf("[FAIL] Allocations overlap or are out of order!\n");
        }
    } else {
        if()
        return;
    }

    // Free the second allocation and reallocate a smaller one
    kfree(ptr2);
    printf("[TEST] Freed second allocation at %p\n", ptr2);

    void* ptr4 = kmalloc(64);
    printf("[TEST] Allocated 64 bytes at %p\n", ptr4);

    // Check if freed memory is reused
    if (ptr4 == ptr2) {
        printf("[PASS] Freed memory was reused correctly.\n");
    } else {
        printf("[FAIL] Freed memory was not reused properly!\n");
    }

    // Free all allocations
    kfree(ptr1);
    kfree(ptr3);
    kfree(ptr4);
    printf("[TEST] Freed all allocations.\n");

    // Check merging
    void* ptr5 = kmalloc(128);
    printf("[TEST] Allocated 128 bytes at %p\n", ptr5);

    if (ptr5 == ptr1) {
        printf("[PASS] Free block merging works correctly.\n");
    } else {
        printf("[FAIL] Free block merging failed!\n");
    }

    printf("[TEST] Heap test completed.\n");
}
