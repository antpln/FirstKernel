#include <kernel/memory.h>
#include <kernel/multiboot.h>

uint32_t* PhysicalMemoryManager::bitmap = nullptr;
uint32_t PhysicalMemoryManager::total_frames = 0;
uint32_t PhysicalMemoryManager::used_frames = 0;

void PhysicalMemoryManager::initialize(uint32_t multiboot_info) {
    auto mb_info = reinterpret_cast<multiboot_info_t*>(multiboot_info);
    uint32_t mem_bytes = (mb_info->mem_upper + 1024) * 1024;
    total_frames = mem_bytes / PAGE_SIZE;
    used_frames = 0;
    uint32_t bitmap_size = total_frames / 32;
    if (total_frames % 32) bitmap_size++;
    // Allocate or reserve space for 'bitmap' array here
}

void* PhysicalMemoryManager::allocate_frame() {
    uint32_t frame = first_free();
    if (frame == UINT32_MAX) {
        return nullptr;  // No free frames available
    }

    set_frame(frame);
    used_frames++;
    return (void*)(frame * PAGE_SIZE); // Return physical address
}


void PhysicalMemoryManager::free_frame(void* frame) {
    uint32_t frame_addr = (uint32_t)frame / PAGE_SIZE;
    clear_frame(frame_addr);
    used_frames--;
}


size_t PhysicalMemoryManager::get_memory_size() {
    return total_frames * PAGE_SIZE;
}

size_t PhysicalMemoryManager::get_free_frames() {
    return total_frames - used_frames;
}

void PhysicalMemoryManager::set_frame(uint32_t frame_addr) {
    uint32_t bit = frame_addr % 32;
    uint32_t idx = frame_addr / 32;
    bitmap[idx] |= (1 << bit);
}

void PhysicalMemoryManager::clear_frame(uint32_t frame_addr) {
    uint32_t bit = frame_addr % 32;
    uint32_t idx = frame_addr / 32;
    bitmap[idx] &= ~(1 << bit);
}

uint32_t PhysicalMemoryManager::test_frame(uint32_t frame_addr) {
    uint32_t bit = frame_addr % 32;
    uint32_t idx = frame_addr / 32;
    return (bitmap[idx] & (1 << bit)) != 0;
}

uint32_t PhysicalMemoryManager::first_free() {
    for (uint32_t i = 0; i < total_frames / 32; i++) {
        if (bitmap[i] != 0xFFFFFFFF) { // Not full
            for (uint32_t j = 0; j < 32; j++) {
                if (!(bitmap[i] & (1 << j))) {
                    return i * 32 + j; // Found free frame
                }
            }
        }
    }
    return UINT32_MAX; // No free frame found
}
