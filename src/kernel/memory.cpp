// PhysicalMemoryManager.cpp
#include <stdint.h>
#include <string.h> // for memset, etc.
#include "kernel/memory.h"
#include "kernel/multiboot.h"     // so we can interpret multiboot_info_t

// In your code, define the kernel_end symbol in the linker script, e.g.:
//   SECTIONS {
//     .text : { ... }
//     .rodata : { ... }
//     .data : { ... }
//     .bss : { ... }
//     . = ALIGN(4);
//     kernel_end = .;
//   }
//
// Then declare it as an extern in C:
extern "C" uint32_t kernel_end;

// Static class members
uint32_t* PhysicalMemoryManager::bitmap       = nullptr;
uint32_t  PhysicalMemoryManager::total_frames = 0;
uint32_t  PhysicalMemoryManager::used_frames  = 0;

static inline uint32_t align_up(uint32_t val, uint32_t align) {
    return (val + (align - 1)) & ~(align - 1);
}

void PhysicalMemoryManager::initialize(uint32_t multiboot_info_addr)
{
    // Interpret the multiboot structure
    auto mb_info = reinterpret_cast<multiboot_info_t*>(multiboot_info_addr);

    // The multiboot field mem_upper is # of KB above 1MB
    // So total memory = (mem_upper + 1024) KB
    // Convert to bytes
    uint32_t mem_bytes = (mb_info->mem_upper + 1024) * 1024;

    // Number of frames = total bytes / page size (commonly 4096)
    total_frames = mem_bytes / PAGE_SIZE;

    // We start with no frames used
    used_frames = 0;

    // Compute the size of the bitmap (in 32-bit words)
    // Each 32-bit word can represent 32 frames
    uint32_t bitmap_size = total_frames / 32;
    if (total_frames % 32) {
        bitmap_size++;
    }

    // Now we need to actually store 'bitmap_size' 32-bit words somewhere.
    // For demonstration, let's place it right after the kernel in memory:
    // 'kernel_end' is a linker symbol marking where the kernel finishes.
    static uint32_t next_free_physical = reinterpret_cast<uint32_t>(&kernel_end);

    // Ensure we align up to page boundary if you like (optional):
    next_free_physical = align_up(next_free_physical, PAGE_SIZE);

    // Point bitmap to that region
    bitmap = reinterpret_cast<uint32_t*>(next_free_physical);

    // Advance next_free_physical by the size needed
    uint32_t bytes_needed = bitmap_size * sizeof(uint32_t);
    next_free_physical += bytes_needed;

    // Clear the bitmap (mark all frames as free initially)
    memset(bitmap, 0, bytes_needed);

    // Optionally, you may want to mark frames used by the kernel
    // or other reserved areas as "used" here, so they're not allocated.
    // For example, all frames from 0 up to next_free_physical / PAGE_SIZE.
    // We'll skip that in this minimal example.

    // Now the PMM is ready to start allocating frames.
}

void* PhysicalMemoryManager::allocate_frame()
{
    // Find the first free frame
    uint32_t frame = first_free();
    if (frame == UINT32_MAX) {
        return nullptr; // No free frames available
    }

    set_frame(frame); // Mark it as used
    used_frames++;
    return reinterpret_cast<void*>(frame * PAGE_SIZE); 
}

void PhysicalMemoryManager::free_frame(void* frame)
{
    // Convert address => frame index
    uint32_t frame_addr = reinterpret_cast<uint32_t>(frame) / PAGE_SIZE;
    // Mark as free
    clear_frame(frame_addr);
    used_frames--;
}

size_t PhysicalMemoryManager::get_memory_size()
{
    // Return total physical memory managed
    return total_frames * PAGE_SIZE;
}

size_t PhysicalMemoryManager::get_free_frames()
{
    // Return how many frames are still free
    return (total_frames - used_frames);
}

void PhysicalMemoryManager::set_frame(uint32_t frame_addr)
{
    uint32_t bit = frame_addr % 32;
    uint32_t idx = frame_addr / 32;
    bitmap[idx] |= (1 << bit);
}

void PhysicalMemoryManager::clear_frame(uint32_t frame_addr)
{
    uint32_t bit = frame_addr % 32;
    uint32_t idx = frame_addr / 32;
    bitmap[idx] &= ~(1 << bit);
}

uint32_t PhysicalMemoryManager::test_frame(uint32_t frame_addr)
{
    uint32_t bit = frame_addr % 32;
    uint32_t idx = frame_addr / 32;
    return (bitmap[idx] & (1 << bit)) != 0;
}

uint32_t PhysicalMemoryManager::first_free()
{
    // Recompute the number of bitmap entries
    uint32_t bm_size = total_frames / 32;
    if (total_frames % 32) {
        bm_size++;
    }

    // Scan each 32-bit word
    for (uint32_t i = 0; i < bm_size; i++) {
        // If it's not fully allocated...
        if (bitmap[i] != 0xFFFFFFFF) {
            // Check each bit
            for (uint32_t j = 0; j < 32; j++) {
                uint32_t mask = 1 << j;
                if ((bitmap[i] & mask) == 0) {
                    // This frame is free
                    uint32_t frame_number = i * 32 + j;
                    if (frame_number < total_frames) {
                        return frame_number;
                    } else {
                        break; // beyond actual total frames
                    }
                }
            }
        }
    }
    return UINT32_MAX; // No free frame found
}
