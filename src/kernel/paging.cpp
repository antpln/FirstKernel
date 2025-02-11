#include <kernel/paging.h>
#include <kernel/isr.h>
#include <kernel/memory.h> // Use PhysicalMemoryManager
#include <string.h>        // memset
#include <stdio.h>         // printf for debugging

// Kernel Page Directory
page_directory_t* kernel_directory = nullptr;
page_directory_t* current_directory = nullptr;

// Page Fault Handler (ISR 14)
extern "C" void page_fault_handler(registers_t* regs) {
    uint32_t faulting_address;
    asm volatile("mov %%cr2, %0" : "=r"(faulting_address));

    printf("\n[PAGE FAULT] Address: 0x%x, Error Code: 0x%x\n", faulting_address, regs->err_code);
    while (1) { asm volatile("hlt"); } // Halt execution
}

// Map a virtual address to a physical address
void vmm_map(uint32_t virtual_addr, uint32_t physical_addr, int user, int rw) {
    uint32_t dir_idx = virtual_addr >> 22;   // Top 10 bits
    uint32_t table_idx = (virtual_addr >> 12) & 0x3FF; // Middle 10 bits

    page_table_t* table = kernel_directory->tables[dir_idx];

    // Allocate page table if needed
    if (!table) {
        table = (page_table_t*) PhysicalMemoryManager::allocate_frame();
        memset(table, 0, sizeof(page_table_t));
        kernel_directory->tables[dir_idx] = table;
        kernel_directory->tables_physical[dir_idx] = (uint32_t)table | 3; // Present + RW
    }

    // Set page entry
    table->pages[table_idx].present = 1;
    table->pages[table_idx].rw = rw;
    table->pages[table_idx].user = user;
    table->pages[table_idx].frame_addr = physical_addr >> 12;
}

// Enable paging in CR0
void vmm_enable() {
    asm volatile("mov %0, %%cr3":: "r"(kernel_directory->physical_addr));

    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000; // Set PG (Paging) bit
    asm volatile("mov %0, %%cr0":: "r"(cr0));
}

// Initialize Paging and Identity Map Kernel Memory
void vmm_init() {
    printf("Initializing Paging...\n");

    kernel_directory = (page_directory_t*) PhysicalMemoryManager::allocate_frame();
    memset(kernel_directory, 0, sizeof(page_directory_t));

    // Identity map first 16MB (kernel space)
    for (uint32_t i = 0; i < 16 * 1024 * 1024; i += PAGE_SIZE) {
        vmm_map(i, i, 0, 1); // Kernel mode, Read/Write
    }

    current_directory = kernel_directory;
    vmm_enable();

    // Register page fault handler
    register_interrupt_handler(14, page_fault_handler);

    printf("Paging enabled!\n");
}
