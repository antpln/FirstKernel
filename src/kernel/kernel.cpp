#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "kernel/vga.h"
#include "kernel/idt.h"
#include "kernel/memory.h"
#include "kernel/tests/memtest.h"
#include "kernel/tests/pagetest.h"
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

		// Step 1: Initialize the Physical Memory Manager (PMM)
		printf("[INIT] Initializing Physical Memory Manager...\n");
		PhysicalMemoryManager::initialize(multiboot_info);
	
		// Step 2: Initialize Virtual Memory (Paging)
		printf("[INIT] Initializing Virtual Memory Manager...\n");
		vmm_init();
	
		// Step 3: Enable Interrupts (Only after paging is stable)
		printf("[INIT] Enabling Interrupts...\n");
		asm volatile("sti"); // Enable interrupts
	
		// Step 4: Run Tests (Verify paging)
		printf("[TEST] Running Paging Test...\n");

		terminal.writestring("\nRunning memory tests...\n");

		if (MemoryTester::test_allocation())
		{
			writeSuccess("Basic allocation test passed\n");
		}
		else
		{
			writeError("Basic allocation test failed\n");
		}

		if (MemoryTester::test_free())
		{
			writeSuccess("Memory free test passed\n");
		}
		else
		{
			writeError("Memory free test failed\n");
		}

		if (MemoryTester::test_multiple_allocations())
		{
			writeSuccess("Multiple allocations test passed\n");
		}
		else
		{
			writeError("Multiple allocations test failed\n");
		}
		printf("Memory size is %d\n", PhysicalMemoryManager::get_memory_size());

		paging_test();

		terminal.writestring("Kernel initialization complete\n");

		terminal.setcolor(terminal.make_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
		terminal.writestring("Good evening...\n");
		terminal.setcolor(terminal.make_color(VGA_COLOR_BROWN, VGA_COLOR_BLACK));
		terminal.writestring(ascii_guitar);
	}

#ifdef __cplusplus
}
#endif
