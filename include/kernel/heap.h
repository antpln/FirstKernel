#ifndef HEAP_H
#define HEAP_H

#include <stdint.h>
#include <stddef.h>

#define KERNEL_HEAP_START  0x1000000  // Start of the kernel heap (16MB)
#define KERNEL_HEAP_SIZE   0x400000   // 4MB heap size

void heap_init();
void* kmalloc(size_t size);
void kfree(void* ptr);

#endif
