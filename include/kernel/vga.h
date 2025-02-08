#ifndef _KERNEL_VGA_H
#define _KERNEL_VGA_H

#include <stdint.h>
#include <stddef.h>

enum vga_color {
    // ...existing vga_color enum...
};

#ifdef __cplusplus
extern "C" {
#endif

void terminal_initialize(void);
void terminal_setcolor(uint8_t color);
void terminal_setfull_color(enum vga_color fg, enum vga_color bg);
void terminal_reset_color(void);
void terminal_writestring(const char* data);

#ifdef __cplusplus
}
#endif

#endif
