#include <kernel/isr.h>
#include <stdio.h>
#include <kernel/port_io.h>

#define ISR_COUNT 256 // Total number of ISRs

// Array of function pointers to handle interrupts
static isr_t interrupt_handlers[ISR_COUNT];

// Registers a custom ISR handler for a given interrupt
void register_interrupt_handler(uint8_t n, isr_t handler) {
    interrupt_handlers[n] = handler;
}

// The common ISR handler, called by the assembly stub
extern "C" void isr_handler(registers_t* regs) {
    if (interrupt_handlers[regs->int_no]) {
        isr_t handler = interrupt_handlers[regs->int_no];
        handler(regs);
    } else {
        printf("[ISR] Unhandled interrupt: %d\n", regs->int_no);
    }
}

// IRQ handler (for hardware interrupts)
extern "C" void irq_handler(registers_t* regs) {
    // Send End Of Interrupt (EOI) to PIC
    if (regs->int_no >= 40) {
        // Send reset signal to slave PIC
        outb(0xA0, 0x20);
    }
    // Send reset signal to master PIC
    outb(0x20, 0x20);

    if (interrupt_handlers[regs->int_no]) {
        isr_t handler = interrupt_handlers[regs->int_no];
        handler(regs);
    }
}
