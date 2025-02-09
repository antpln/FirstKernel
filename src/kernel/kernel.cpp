#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "kernel/vga.h"
#include "kernel/idt.h"
#include "utils.h"

#ifdef __cplusplus
extern "C"

{
#endif

	Terminal terminal;
	void writeSuccess(char* message) {
		terminal.setcolor(terminal.make_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
		terminal.writestring(message);

	}
	void writeError(char* message)  {
		terminal.setcolor(terminal.make_color(VGA_COLOR_RED, VGA_COLOR_BLACK));
		terminal.writestring(message);
	}

	void kernel_main(void) 
	{
		char* ascii_guitar = R"(
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

		 // Initialize IDT
    	init_idt();
    
    	writeSuccess("IDT initialized\n");
	}

#ifdef __cplusplus
}
#endif
