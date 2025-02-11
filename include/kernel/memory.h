#ifndef KERNEL_MEMORY_H
#define KERNEL_MEMORY_H

#include <stdint.h>
#include <stddef.h>

class PhysicalMemoryManager {
public:
    static void initialize(uint32_t multiboot_info);
    static void* allocate_frame();
    static void free_frame(void* frame);
    static size_t get_memory_size();
    static size_t get_free_frames();

private:
    static uint32_t* bitmap;
    static uint32_t total_frames;
    static uint32_t used_frames;
    
    static void set_frame(uint32_t frame_addr);
    static void clear_frame(uint32_t frame_addr);
    static uint32_t test_frame(uint32_t frame_addr);
    static uint32_t first_free();
};

#define PAGE_SIZE 4096

#endif