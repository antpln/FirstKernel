#include <kernel/heap.h>
#include <kernel/memory.h>  // Use your existing physical memory manager
#include <stddef.h>
#include <stdint.h>

// Heap block header structure.
typedef struct heap_block {
    size_t size;                 // Size of the block (excluding the header)
    struct heap_block* next;     // Pointer to the next block in the free list
    uint8_t free;                // 1 if the block is free, 0 if it's allocated
} heap_block_t;

static heap_block_t* free_list = NULL;  // Start of the free list

// Align the requested size to a 4-byte boundary.
static size_t align4(size_t size) {
    return (size + 3) & ~((size_t)3);
}

void init_heap() {
    // Initialize the heap with a single large free block.
    free_list = (heap_block_t*)KERNEL_HEAP_START;
    // The entire heap minus the header is available.
    free_list->size = KERNEL_HEAP_SIZE - sizeof(heap_block_t);
    free_list->next = NULL;
    free_list->free = 1;
}

void* kmalloc(size_t size) {
    if (size == 0) {
        return NULL;
    }
    size = align4(size);

    heap_block_t* current = free_list;
    heap_block_t* previous = NULL;
    // Find a free block large enough to satisfy the request.
    while (current != NULL) {
        if (current->free && current->size >= size) {
            break;
        }
        previous = current;
        current = current->next;
    }

    if (current == NULL) {
        // No suitable block found.
        return NULL;
    }

    // If the block is large enough to split into two parts,
    // ensure the remaining part is large enough to hold a header plus minimal payload.
    if (current->size >= size + sizeof(heap_block_t) + 4) {
        uintptr_t block_addr = (uintptr_t)current;
        heap_block_t* new_block = (heap_block_t*)(block_addr + sizeof(heap_block_t) + size);
        new_block->size = current->size - size - sizeof(heap_block_t);
        new_block->next = current->next;
        new_block->free = 1;
        current->size = size;
        current->next = new_block;
    }

    // Mark the block as allocated.
    current->free = 0;
    // Return the memory just past the header.
    return (void*)((uintptr_t)current + sizeof(heap_block_t));
}

void kfree(void* ptr) {
    if (!ptr) return;

    // Get the block header from the pointer.
    heap_block_t* block = (heap_block_t*)((uintptr_t)ptr - sizeof(heap_block_t));
    block->free = 1;

    // Coalesce adjacent free blocks.
    heap_block_t* current = free_list;
    while (current != NULL) {
        if (current->free && current->next && current->next->free) {
            current->size += current->next->size + sizeof(heap_block_t);
            current->next = current->next->next;
            // Re-check current block's next pointer after merging.
            continue;
        }
        current = current->next;
    }
}
