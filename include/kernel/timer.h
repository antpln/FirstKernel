#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include "kernel/isr.h"  // For registers_t

// Initializes the PIT timer to the specified frequency.
void init_timer(uint32_t frequency);

// Timer interrupt handler.
void timer_handler(registers_t* regs);

uint32_t get_ticks();
uint32_t get_ticks_miliseconds();

#endif // TIMER_H
