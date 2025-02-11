#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "kernel/vga.h"
#include "kernel/idt.h"
#include "kernel/memory.h"
#include "utils.h"

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

		// Initialize IDT
		init_idt();
		writeSuccess("IDT initialized\n");

		// Initialize memory management
		PhysicalMemoryManager::initialize(multiboot_info);
		writeSuccess("Physical Memory Manager initialized\n");

		// Test memory allocation
		void *frame = PhysicalMemoryManager::allocate_frame();
		if (frame)
		{
			writeSuccess("Successfully allocated memory frame\n");
		}
		else
		{
			writeError("Failed to allocate memory frame\n");
		}

		terminal.writestring("Kernel initialization complete\n");

		terminal.setcolor(terminal.make_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
		terminal.writestring("Good evening...\n");
		terminal.setcolor(terminal.make_color(VGA_COLOR_BROWN, VGA_COLOR_BLACK));
		terminal.writestring(ascii_guitar);
	}

#ifdef __cplusplus
}
#endif
