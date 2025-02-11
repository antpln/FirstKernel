// paging.cpp
#include "kernel/paging.h"
#include <stdint.h>
#include <string.h>         // For memset
#include <kernel/memory.h>  // For PhysicalMemoryManager

// Define the offset between the kernel's virtual addresses and physical addresses.
// For example, if your kernel is loaded at 0xC0000000, then:
#define KERNEL_OFFSET 0x0


// Since we cannot use kmalloc, we allocate a static kernel page directory.
static page_directory_t kernel_directory_instance;
page_directory_t* kernel_directory = &kernel_directory_instance;
page_directory_t* current_directory = &kernel_directory_instance;

// Helper function: Given a virtual address, return its corresponding page_t entry.
// If 'make' is nonzero and no page table exists, allocate one.
static page_t* get_page(uint32_t address, int make, page_directory_t* dir) {
    // Calculate indices: upper 10 bits index the page directory,
    // the next 10 bits index the page table.
    uint32_t pd_index = address >> 22;
    uint32_t pt_index = (address >> 12) & 0x03FF;

    // If there is no page table at this slot, create one if requested.
    if (!dir->tables[pd_index]) {
        if (!make)
            return 0;

        // Allocate one physical frame for the new page table.
        // PhysicalMemoryManager::allocate_frame() returns a physical address.
        uint32_t phys_table_addr = (uint32_t)PhysicalMemoryManager::allocate_frame();
        if (!phys_table_addr)
            return 0; // Allocation failure

        // Convert the physical address to a kernel–accessible virtual address.
        // This is safe because we've identity–mapped the first 16 MB.
        page_table_t* table = (page_table_t*)(phys_table_addr + KERNEL_OFFSET);
        memset(table, 0, sizeof(page_table_t));

        // Save the page table pointer in the directory.
        dir->tables[pd_index] = table;
        // Record its physical address (without the offset).
        dir->tables_physical[pd_index] = phys_table_addr;
    }

    // Return a pointer to the specific page within the table.
    return &dir->tables[pd_index]->pages[pt_index];
}

// Map the virtual address 'virtual_addr' to the physical address 'physical_addr'.
// The 'user' flag indicates whether the page is accessible from user-mode,
// and the 'rw' flag indicates read/write permissions.
void vmm_map(uint32_t virtual_addr, uint32_t physical_addr, int user, int rw) {
    page_t* page = get_page(virtual_addr, 1, kernel_directory);
    if (!page) {
        // Handle allocation error if desired.
        return;
    }

    page->present    = 1;              // Mark page as present.
    page->rw         = (rw ? 1 : 0);     // Set read/write flag.
    page->user       = (user ? 1 : 0);     // Set user/supervisor flag.
    page->accessed   = 0;
    page->dirty      = 0;
    page->unused     = 0;
    // Save the physical frame address in the page entry (shifted right 12 bits).
    page->frame_addr = physical_addr >> 12;
}

// Enable paging by loading our page directory into CR3 and then setting the paging bit in CR0.
void vmm_enable() {
    // CR3 must be loaded with the physical address of the page directory.
    // Our kernel_directory->physical_addr field must be set correctly.
    asm volatile("mov %0, %%cr3" : : "r" (kernel_directory->physical_addr));

    // Set the paging (PG) bit (bit 31) in CR0.
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r" (cr0));
    cr0 |= 0x80000000;  // Set bit 31.
    asm volatile("mov %0, %%cr0" : : "r" (cr0));
}

// Initialize our paging system.
void vmm_init() {
    // Zero out the kernel directory.
    memset(kernel_directory, 0, sizeof(page_directory_t));
    for (int i = 0; i < 1024; i++) {
        kernel_directory->tables[i] = 0;
        kernel_directory->tables_physical[i] = 0;
    }
    // The page directory's physical address should be the physical address corresponding
    // to its tables_physical array. Since kernel_directory is allocated in the kernel's
    // high-half, subtract KERNEL_OFFSET to convert its virtual address to a physical address.
    kernel_directory->physical_addr = (uint32_t)&kernel_directory->tables_physical - KERNEL_OFFSET;

    // Identity map the first 16 MB of memory.
    // This ensures that low memory (including our page tables) remains accessible.
    for (uint32_t addr = 0; addr < 0x1000000; addr += PAGE_SIZE) {
        // Map with user=0 (kernel–only) and rw=1 (read/write).
        vmm_map(addr, addr, 0, 1);
    }
}
