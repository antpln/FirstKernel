#include "kernel/paging.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <kernel/isr.h>

// Define heap boundaries to avoid conflicts with paging
#define KERNEL_HEAP_START 0x00800000  // Heap starts at 8 MiB
#define KERNEL_HEAP_SIZE  0x00800000  // Heap spans 8 MiB (up to 16 MiB)

// Page directory (1024 entries) and four page tables (each 4 MiB)
static uint32_t kernel_page_directory[1024]
    __attribute__((aligned(4096), section(".lowmem")));

static uint32_t kernel_page_table0[1024]
    __attribute__((aligned(4096), section(".lowmem")));

static uint32_t kernel_page_table1[1024]
    __attribute__((aligned(4096), section(".lowmem")));

static uint32_t kernel_page_table2[1024]
    __attribute__((aligned(4096), section(".lowmem")));

static uint32_t kernel_page_table3[1024]
    __attribute__((aligned(4096), section(".lowmem")));

// Page fault handler
void page_fault_handler(registers_t *registers) {
    uint32_t fault_addr;
    asm("mov %%cr2, %0" : "=r"(fault_addr));
    
    printf("[VMM] Page Fault at 0x%x\n", fault_addr);
    printf("[VMM] Page info: 0x%x\n", registers->eip);
    printf("[VMM] Page fault caused by %s access\n",
           (registers->err_code & 0x1) ? "write" : "read");
    printf("[VMM] Page fault %s\n",
           (registers->err_code & 0x2) ? "protection" : "non-present");
    printf("[VMM] Page fault in %s mode\n",
           (registers->err_code & 0x4) ? "user" : "supervisor");
    printf("[VMM] Page fault caused by %s operation\n",
           (registers->err_code & 0x8) ? "instruction fetch" : "data access");

    for (;;) asm("hlt");
}

void vmm_init()
{
    printf("[VMM] Initializing paging (identity map 0..16 MiB)\n");

    // Register the page fault handler
    register_interrupt_handler(14, page_fault_handler);

    // 1) Clear the page directory and tables
    memset(kernel_page_directory, 0, sizeof(kernel_page_directory));
    memset(kernel_page_table0, 0, sizeof(kernel_page_table0));
    memset(kernel_page_table1, 0, sizeof(kernel_page_table1));
    memset(kernel_page_table2, 0, sizeof(kernel_page_table2));
    memset(kernel_page_table3, 0, sizeof(kernel_page_table3));

    // 2) Fill the four page tables (0–4MiB, 4–8MiB, 8–12MiB, 12–16MiB)
    printf("[VMM] Mapping [0..16 MiB]\n");
    uint32_t* tables[] = {kernel_page_table0, kernel_page_table1, kernel_page_table2, kernel_page_table3};
    for (int table_idx = 0; table_idx < 4; table_idx++) {
        for (uint32_t i = 0; i < 1024; i++) {
            uint32_t phys_addr = (table_idx * 0x400000) + (i * 0x1000);
            tables[table_idx][i] = (phys_addr & 0xFFFFF000) | 0x03; // Present + RW
        }
    }

    // 3) Map the page tables in the directory
    kernel_page_directory[0] = ((uint32_t)kernel_page_table0 & 0xFFFFF000) | 0x03;
    kernel_page_directory[1] = ((uint32_t)kernel_page_table1 & 0xFFFFF000) | 0x03;
    kernel_page_directory[2] = ((uint32_t)kernel_page_table2 & 0xFFFFF000) | 0x03;
    kernel_page_directory[3] = ((uint32_t)kernel_page_table3 & 0xFFFFF000) | 0x03;

    printf("[VMM] PDE[0] = 0x%x\n", kernel_page_directory[0]);
    printf("[VMM] PDE[1] = 0x%x\n", kernel_page_directory[1]);
    printf("[VMM] PDE[2] = 0x%x\n", kernel_page_directory[2]);
    printf("[VMM] PDE[3] = 0x%x\n", kernel_page_directory[3]);

    // Print first few entries for debugging
    printf("[VMM] First 4 entries of page_table0:\n");
    for (int i = 0; i < 4; i++) {
        printf("  PT0[%d] = 0x%x\n", i, kernel_page_table0[i]);
    }
    
    printf("[VMM] PDE @ 0x%x\n", (uint32_t)kernel_page_directory);
}

void vmm_enable()
{
    printf("[VMM] Enabling paging...\n");

    asm volatile("cli");

    // Load CR3 (physical address of page directory)
    uint32_t pde_phys = (uint32_t)kernel_page_directory;
    printf("[VMM] Loading CR3 with 0x%x\n", pde_phys);
    asm volatile("mov %0, %%cr3" :: "r"(pde_phys));

    // Enable paging in CR0
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    printf("[VMM] Old CR0 = 0x%x\n", cr0);

    cr0 |= 0x80000000;  // Set PG bit
    cr0 |= 0x00000001;  // Ensure PE bit is set
    printf("[VMM] New CR0 = 0x%x\n", cr0);

    asm volatile("mov %0, %%cr0" :: "r"(cr0) : "memory");

    // Optional far jump
    asm volatile(
        "ljmp $0x08, $1f\n"
        "1:\n"
    );

    printf("[VMM] Paging enabled successfully.\n");
}

void vmm_map(uint32_t virtual_addr, uint32_t physical_addr, int rw)
{
    printf("[VMM] Mapping vaddr=0x%x to paddr=0x%x, rw=%d\n", virtual_addr, physical_addr, rw);

    uint32_t pd_index = (virtual_addr >> 22) & 0x3FF;
    uint32_t pt_index = (virtual_addr >> 12) & 0x3FF;

    uint32_t pde_val = kernel_page_directory[pd_index];
    if ((pde_val & 1) == 0) {
        printf("[VMM] PDE[%d] not present!\n", pd_index);
        return;
    }

    uint32_t pt_phys_base = pde_val & 0xFFFFF000;
    uint32_t* pt_virt_base = (uint32_t*)pt_phys_base; // Identity-mapped

    uint32_t flags = (rw ? 0x3 : 0x1);
    pt_virt_base[pt_index] = (physical_addr & 0xFFFFF000) | flags;

    printf("[VMM] PT[%d] = 0x%x\n", pt_index, pt_virt_base[pt_index]);

    asm volatile("invlpg (%0)" :: "r"(virtual_addr) : "memory");
    printf("[VMM] Mapping done.\n");
}
