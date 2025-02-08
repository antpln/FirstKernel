#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "kernel/vga.h"
#include "utils.h"

#ifdef __cplusplus
extern "C"
{
#endif

	extern "C" void kernel_main(void) 
	{
		char* ascii_guitar = R"(
          Q
         /|\
       (o\_)=="#
        \| |\
       ~H| |/
            ~)";
		Terminal terminal;
		terminal.initialize();
		terminal.writestring("Good evening ...");
		terminal.setcolor(terminal.make_color(VGA_COLOR_BROWN, VGA_COLOR_BLACK));
		terminal.writestring(ascii_guitar);
	}

#ifdef __cplusplus
}
#endif
