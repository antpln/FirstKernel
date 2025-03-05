#include <stdint.h>
#include <string.h> // for memset
#include "kernel/memory.h"
#include "kernel/multiboot.h" // for multiboot_info_t

extern "C" uint32_t kernel_end;

uint32_t* PhysicalMemoryManager::bitmap       = nullptr;
uint32_t  PhysicalMemoryManager::total_frames = 0;
uint32_t  PhysicalMemoryManager::used_frames  = 0;

/* Helper: Align 'val' up to 'align' boundary */
static inline uint32_t align_up(uint32_t val, uint32_t align) {
    return (val + (align - 1)) & ~(align - 1);
}

void PhysicalMemoryManager::initialize(uint32_t multiboot_info_addr)
{
    // 1) Interpret multiboot structure
    auto mb_info = reinterpret_cast<multiboot_info_t*>(multiboot_info_addr);

    // 2) Determine total memory using mem_upper
    //    mem_upper is KB above 1MB. So total memory = (mem_upper + 1024) KB
    //    Convert to bytes, then to page frames
    uint32_t mem_bytes = (mb_info->mem_upper + 1024) * 1024;
    total_frames = mem_bytes / PAGE_SIZE;

    used_frames = 0;

    // 3) Compute bitmap size in 32-bit words
    uint32_t bitmap_size = total_frames / 32;
    if (total_frames % 32) {
        bitmap_size++;
    }

    // 4) Place the bitmap just after the kernel in memory
    static uint32_t next_free_physical = reinterpret_cast<uint32_t>(&kernel_end);
    next_free_physical = align_up(next_free_physical, PAGE_SIZE);

    bitmap = reinterpret_cast<uint32_t*>(next_free_physical);

    uint32_t bytes_needed = bitmap_size * sizeof(uint32_t);
    next_free_physical += bytes_needed;

    // 5) Clear the bitmap (mark all frames as free initially)
    memset(bitmap, 0, bytes_needed);

    // 6) IMPORTANT: Mark [0 .. next_free_physical) as used,
    //    since this area contains the kernel + this bitmap itself.
    for (uint32_t addr = 0; addr < next_free_physical; addr += PAGE_SIZE) {
        uint32_t frame_idx = addr / PAGE_SIZE;
        set_frame(frame_idx);  // mark frame as used
        used_frames++;
    }

    // (Optionally, if you know other regions are reserved, mark them used too.)
}

void* PhysicalMemoryManager::allocate_frame()
{
    uint32_t frame = first_free();
    if (frame == UINT32_MAX) {
        return nullptr; // no free frames available
    }
    set_frame(frame);
    used_frames++;
    // Return the physical address of this frame
    return reinterpret_cast<void*>(frame * PAGE_SIZE);
}

void PhysicalMemoryManager::free_frame(void* frame)
{
    // Convert address => frame index
    uint32_t frame_idx = reinterpret_cast<uint32_t>(frame) / PAGE_SIZE;
    clear_frame(frame_idx);
    used_frames--;
}

size_t PhysicalMemoryManager::get_memory_size()
{
    return total_frames * PAGE_SIZE;
}

size_t PhysicalMemoryManager::get_free_frames()
{
    return total_frames - used_frames;
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
    // number of 32-bit entries in the bitmap
    uint32_t bm_size = total_frames / 32;
    if (total_frames % 32) {
        bm_size++;
    }
    // Find the first zero bit
    for (uint32_t i = 0; i < bm_size; i++) {
        if (bitmap[i] != 0xFFFFFFFF) {
            for (uint32_t j = 0; j < 32; j++) {
                uint32_t mask = (1 << j);
                if ((bitmap[i] & mask) == 0) {
                    // free frame found
                    uint32_t frame_number = i * 32 + j;
                    if (frame_number < total_frames) {
                        return frame_number;
                    } else {
                        break; // beyond total_frames
                    }
                }
            }
        }
    }
    return UINT32_MAX; // no free frame
}
