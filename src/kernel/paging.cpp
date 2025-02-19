// paging.cpp
#include "kernel/paging.h"
#include <stdint.h>
#include <string.h>         // For memset
#include <kernel/memory.h>  // For PhysicalMemoryManager
#include <stdio.h>          // For printf

// We assume our kernel is loaded at 2MB. This is the offset between the kernel’s
// virtual addresses and physical addresses.
#define KERNEL_OFFSET 0x200000

// ***********************************************
// IMPORTANT:
// To use our simple identity mapping, we must guarantee that
// all our paging structures (page directory and static page tables)
// lie in the first 8MB (which we identity-map in vmm_init).
// One method is to force these structures into a special section (here ".lowmem")
// which you then tell your linker script to locate below 8MB.
// ***********************************************

// Static page tables for the first 8MB (two page tables: one for 0-4MB and one for 4-8MB)
// Place them in the ".lowmem" section.
static page_table_t kernel_page_tables[2] __attribute__((aligned(4096), section(".lowmem")));

// Static instance of a page directory, also forced into low memory.
static page_directory_t kernel_directory_instance __attribute__((aligned(4096), section(".lowmem")));

// Since we cannot use kmalloc here, we use our static page directory.
page_directory_t* kernel_directory = &kernel_directory_instance;
page_directory_t* current_directory = &kernel_directory_instance;

// Given a virtual address, returns a pointer to its page_t entry in the page directory 'dir'.
// If 'make' is nonzero and no page table exists, a new page table is allocated.
static page_t* get_page(uint32_t address, int make, page_directory_t* dir) {
    // The top 10 bits index into the page directory,
    // the next 10 bits index into the page table.
    uint32_t pd_index = address >> 22;
    uint32_t pt_index = (address >> 12) & 0x03FF;

    // If there is no page table at this directory entry, create one if requested.
    if (!dir->tables[pd_index]) {
        if (!make)
            return 0;

        // Allocate one physical frame for the new page table.
        // This function returns a physical address.
        uint32_t phys_table_addr = (uint32_t)PhysicalMemoryManager::allocate_frame();
        if (!phys_table_addr)
            return 0; // Allocation failure

        // Convert the physical address to a kernel–accessible virtual address.
        // (This is safe if that frame is in the identity–mapped region.)
        page_table_t* table = (page_table_t*)(phys_table_addr + KERNEL_OFFSET);
        memset(table, 0, sizeof(page_table_t));

        // Save the pointer and record its physical address.
        dir->tables[pd_index] = table;
        dir->tables_physical[pd_index] = phys_table_addr;
    }

    // Return the address of the requested page entry.
    return &dir->tables[pd_index]->pages[pt_index];
}

// Map the virtual address 'virtual_addr' to the physical address 'physical_addr'.
// 'user' indicates whether the page is user–accessible and 'rw' whether it is writable.
void vmm_map(uint32_t virtual_addr, uint32_t physical_addr, int user, int rw) {
    page_t* page = get_page(virtual_addr, 1, kernel_directory);
    if (!page) {
        // Allocation error handling (if desired)
        return;
    }

    page->present    = 1;              // Mark page as present.
    page->rw         = (rw ? 1 : 0);     // Read/write flag.
    page->user       = (user ? 1 : 0);     // User/supervisor flag.
    page->accessed   = 0;
    page->dirty      = 0;
    page->unused     = 0;
    // Store the physical frame address (shifted right 12 bits).
    page->frame_addr = physical_addr >> 12;
}

// Enable paging by loading our page directory into CR3 and setting the paging bit in CR0.
// Note: The far jump below is used to flush the prefetch queue.
// It also relies on your GDT being correctly set up so that 0x08 is a valid code segment.
void vmm_enable() {
    printf("[VMM] Enabling paging...\n");

    // Disable interrupts.
    asm volatile("cli");

    // Enable Page Size Extension (PSE) in CR4.
    uint32_t cr4;
    asm volatile("mov %%cr4, %0" : "=r"(cr4));
    cr4 |= 0x10;  // Set the PSE bit.
    asm volatile("mov %0, %%cr4" :: "r"(cr4));

    // Load CR3 with the physical address of our page directory.
    // (kernel_directory->physical_addr must point to a low-memory address.)
    printf("[VMM] Loading CR3 with 0x%x\n", kernel_directory->physical_addr);
    asm volatile("mov %0, %%cr3" :: "r"(kernel_directory->physical_addr));

    // Enable paging (set PG bit) and ensure we’re in protected mode (PE bit) in CR0.
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;  // Set the PG (paging) bit.
    cr0 |= 0x00000001;  // Set the PE (protected mode) bit.
    printf("[VMM] Setting CR0 to 0x%x\n", cr0);
    asm volatile("mov %0, %%cr0" :: "r"(cr0) : "memory");

    // Perform a far jump to flush the prefetch queue.
    // Ensure your GDT is set up such that 0x08 is a valid kernel code segment.
    asm volatile(
        "ljmp $0x08, $1f\n"
        "1:\n"
    );

    // (Optionally, re-enable interrupts here.)
    // asm volatile("sti");

    printf("[VMM] Paging enabled successfully\n");
}

// Initialize the paging system.
void vmm_init() {
    printf("[VMM] Initializing paging...\n");

    // Clear the page tables and directory.
    memset(kernel_page_tables, 0, sizeof(kernel_page_tables));
    memset(&kernel_directory_instance, 0, sizeof(page_directory_t));

    // Set up an identity mapping for the first 8MB using our static page tables.
    // (This will cover 0-4MB and 4-8MB.)
    for (int table = 0; table < 2; table++) {
        for (uint32_t i = 0; i < 1024; i++) {
            kernel_page_tables[table].pages[i].present = 1;
            kernel_page_tables[table].pages[i].rw = 1;
            kernel_page_tables[table].pages[i].user = 0;
            // Map each 4KB page: virtual = physical.
            kernel_page_tables[table].pages[i].frame_addr = i + (table * 1024);
        }

        // Add the page table to the directory.
        kernel_directory->tables[table] = &kernel_page_tables[table];
        // IMPORTANT: Store the physical address of the page table.
        // Since our page tables are in low memory (but referenced by a virtual address),
        // we convert by subtracting KERNEL_OFFSET.
        kernel_directory->tables_physical[table] =
            ((uint32_t)&kernel_page_tables[table]) - KERNEL_OFFSET;
    }

    // Set the physical address for the page directory.
    kernel_directory->physical_addr = ((uint32_t)kernel_directory) - KERNEL_OFFSET;
    printf("[VMM] Page directory at physical address 0x%x\n", kernel_directory->physical_addr);
}
