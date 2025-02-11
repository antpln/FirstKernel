#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

#define PAGE_SIZE 4096  // 4 KB Pages

// Page table entry structure
typedef struct {
    uint32_t present    : 1;  // Page present in memory
    uint32_t rw         : 1;  // Read/write
    uint32_t user       : 1;  // User/supervisor mode
    uint32_t accessed   : 1;  // Accessed (set by CPU)
    uint32_t dirty      : 1;  // Written to (set by CPU)
    uint32_t unused     : 7;  // Unused/reserved
    uint32_t frame_addr : 20; // Frame address (shifted right 12 bits)
} page_t;

// Page table structure
typedef struct {
    page_t pages[1024];
} page_table_t;

// Page directory structure
typedef struct {
    page_table_t* tables[1024]; // Array of pointers to page tables
    uint32_t tables_physical[1024]; // Physical addresses of page tables
    uint32_t physical_addr; // Physical address of tables_physical
} page_directory_t;

extern page_directory_t* kernel_directory;
extern page_directory_t* current_directory;

void vmm_init();
void vmm_map(uint32_t virtual_addr, uint32_t physical_addr, int user, int rw);
void vmm_enable();

#endif
