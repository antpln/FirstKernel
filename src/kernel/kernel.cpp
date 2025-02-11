#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "kernel/vga.h"
#include "kernel/idt.h"
#include "kernel/memory.h"
#include "kernel/paging.h"
#include "kernel/idt.h"
#include "kernel/heap.h"
#include "kernel/serial.h"
#include "kernel/tests/memtest.h"
#include "kernel/tests/pagetest.h"
#include "kernel/tests/heaptest.h"
#include "utils.h"
#include <stdio.h>  // Changed back to just stdio.h since include path is set in Makefile


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
		terminal.writestring("Good evening ...");
		terminal.setcolor(terminal.make_color(VGA_COLOR_BROWN, VGA_COLOR_BLACK));
		terminal.writestring(ascii_guitar);
		terminal.writestring("\n");

		terminal.initialize();
		terminal.writestring("Initializing kernel...\n");

		printf("[INIT] Initializing Physical Memory Manager...\n");
		PhysicalMemoryManager::initialize(multiboot_info);
	
		printf("[INIT] Initializing Virtual Memory Manager...\n");
		vmm_init();
	
		printf("[INIT] Enabling Interrupts...\n");
		asm volatile("sti"); // Enable interrupts

		printf("[INIT] Initializing IDT...\n");
		init_idt();

		printf("[INIT] Initializing kernel heap...\n");
		heap_init();

		printf("[INIT] Initializing serial port...\n");
		serial_init();

		

		terminal.writestring("Kernel initialization complete\n");

		terminal.setcolor(terminal.make_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
		terminal.writestring("Good evening...\n");
		terminal.setcolor(terminal.make_color(VGA_COLOR_BROWN, VGA_COLOR_BLACK));
		terminal.writestring(ascii_guitar);



	}

#ifdef __cplusplus
}
#endif
