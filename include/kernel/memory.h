#ifndef PHYSICAL_MEMORY_MANAGER_H
#define PHYSICAL_MEMORY_MANAGER_H

#include <stddef.h>
#include <stdint.h>

#define PAGE_SIZE 4096

class PhysicalMemoryManager {
public:
    static void initialize(uint32_t multiboot_info_addr);

    static void* allocate_frame();
    static void free_frame(void* frame);

    static size_t get_memory_size();
    static size_t get_free_frames();

    static void set_frame(uint32_t frame_addr);
    static void clear_frame(uint32_t frame_addr);
    static uint32_t test_frame(uint32_t frame_addr);
    static uint32_t first_free();
    static uint32_t used_frames;


private:
    static uint32_t* bitmap;
    static uint32_t total_frames;



};
#endif
