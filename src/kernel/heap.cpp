#include <kernel/heap.h>
#include <kernel/memory.h>  // Use your existing physical memory manager
#include <stddef.h>

typedef struct heap_block {
    size_t size;                 // Size of the block (including this header)
    struct heap_block* next;     // Pointer to the next free block
    uint8_t free;                // 1 if the block is free, 0 if it's allocated
} heap_block_t;

static heap_block_t* free_list = NULL;  // Free list of memory blocks
static uint32_t heap_end = KERNEL_HEAP_START;  // Track heap growth

void init_heap() {
    // Initialize the heap with a single large free block
    free_list = (heap_block_t*)KERNEL_HEAP_START;
    free_list->size = KERNEL_HEAP_SIZE - sizeof(heap_block_t);
    free_list->next = NULL;
    free_list->free = 1;
}

void* kmalloc(size_t size) {
    // Find a free block that can accommodate the requested size
    heap_block_t* current = free_list;
    heap_block_t* previous = NULL;
    while (current != NULL) {
        if (current->size >= size && current->free) {
            break;
        }
        previous = current;
        current = current->next;
    }

    // If no block is found, return NULL
    if (current == NULL) {
        return NULL;
    }

    // If the block is large enough to split, create a new block
    if (current->size > size + sizeof(heap_block_t)) {
        heap_block_t* new_block = (heap_block_t*)((uint32_t)current + size + sizeof(heap_block_t));
        new_block->size = current->size - size - sizeof(heap_block_t);
        new_block->next = current->next;
        new_block->free = 1;
        current->size = size;
        current->next = new_block;
    }

    // Mark the block as allocated
    current->free = 0;
    return (void*)((uint32_t)current + sizeof(heap_block_t));
}

void kfree(void* ptr) {
    // Mark the block as free
    heap_block_t* block = (heap_block_t*)((uint32_t)ptr - sizeof(heap_block_t));
    block->free = 1;

    // Coalesce adjacent free blocks
    heap_block_t* current = free_list;
    while (current != NULL) {
        if (current->free && current->next != NULL && current->next->free) {
            current->size += current->next->size + sizeof(heap_block_t);
            current->next = current->next->next;
        }
        current = current->next;
    }
}