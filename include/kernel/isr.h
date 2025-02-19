#ifndef _KERNEL_ISR_H
#define _KERNEL_ISR_H

#include <stdint.h>

typedef struct registers {
    uint32_t ds;         // Data segment
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;  // Pushed by pusha
    uint32_t int_no, err_code;  // Interrupt number & error code (manually pushed)
    uint32_t eip, cs, eflags, useresp, ss;  // Automatically pushed by the CPU
} registers_t;


typedef void (*isr_t)(registers_t*);
void register_interrupt_handler(uint8_t n, isr_t handler);

#endif