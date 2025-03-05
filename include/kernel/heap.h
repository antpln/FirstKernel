#ifndef HEAP_H
#define HEAP_H

#include <stdint.h>
#include <stddef.h>

#define KERNEL_HEAP_START  0x00800000  // 8 MiB
#define KERNEL_HEAP_SIZE   0x00800000  // 8 MiB heap (adjustable)

void init_heap();
void* kmalloc(size_t size);
void kfree(void* ptr);
void* krealloc(void* ptr, size_t size);

#endif
