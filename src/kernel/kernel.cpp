#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "kernel/vga.h"
#include "kernel/idt.h"
#include "kernel/memory.h"
#include "kernel/paging.h"
#include "kernel/heap.h"
#include "kernel/pic.h"
#include "kernel/keyboard.h"
#include "kernel/gdt.h"
#include "kernel/timer.h"
#include "kernel/shell.h"
#include "kernel/ramfs.h"
#include "kernel/tests/memtest.h"
#include "kernel/tests/pagetest.h"
#include "kernel/tests/heaptest.h"

#include "utils.h"
#include <stdio.h> // Changed back to just stdio.h since include path is set in Makefile


void check_interrupts()
{
	uint32_t flags;
	__asm__ volatile("pushf; pop %0" : "=r"(flags));

	if (flags & (1 << 9))
	{
		printf("Interrupts are ENABLED.\n");
	}
	else
	{
		printf("Interrupts are DISABLED!\n");
	}
}


void dump_stack() {
    uint32_t *esp;
    asm volatile ("mov %%esp, %0" : "=r" (esp));

    printf("ESP: 0x%x\n", esp);
    for (int i = 0; i < 10; i++) {
        printf("[%d] 0x%x\n", i, esp[i]);
    }
}

void dump_gdt() {
    GDTPtr gdtr;
	asm volatile("sgdt %0" : "=m"(gdtr));
	printf("GDT Base=0x%x, Limit=0x%x\n", gdtr.base, gdtr.limit);
}




#ifdef __cplusplus
extern "C"

{
#endif

	Terminal terminal;
	void writeSuccess(char *message)
	{
		terminal.setcolor(terminal.make_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
		terminal.writestring(message);
	}
	void writeError(char *message)
	{
		terminal.setcolor(terminal.make_color(VGA_COLOR_RED, VGA_COLOR_BLACK));
		terminal.writestring(message);
	}

	void kernel_main(uint32_t multiboot_info)
	{

		char *ascii_guitar = R"(
          Q
         /|\
       (o\_)=="#
        \| |\
       ~H| |/
            ~)";
		terminal.initialize();
		uint32_t cr0;
		asm volatile ("mov %%cr0, %0" : "=r"(cr0)); // Read CR0 register
		if (cr0 & 1) {
			printf("Running in Protected Mode\n");
		} else {
			printf("Running in Real Mode\n");
		}

		init_gdt();  // sets up GDT and flushes it
		//Set up heap
		init_heap();

		// Initialize the RAMFS.
		fs_init();

		// (Optionally) Create some built-in files or directories.
		FSNode* root = fs_get_root();
		FSNode* readme = fs_create_node("README", FS_FILE);
		if (readme) {
			// Allocate a buffer for the file content.
			readme->size = 128;
			readme->data = (uint8_t*)kmalloc(readme->size);
			if (readme->data) {
				// Write some content into the file.
				const char* msg = "Welcome to your kernel RAMFS!";
				strncpy((char*)readme->data, msg, readme->size);
			}
			fs_add_child(root, readme);
		}

		keyboard_install();
		// Initialize the PIT timer to 1000 Hz
		init_timer(1000);

		// Remap the PIC
		init_pic();

		// Initialize the IDT
		init_idt();

		shell_init();



		__asm__ volatile("sti");

		while(1){
			__asm__ volatile("hlt");
		}
	}

#ifdef __cplusplus
}
#endif
