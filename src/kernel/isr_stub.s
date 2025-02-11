.global isr_stub
.global irq_stub

isr_stub:
    cli                     # Disable interrupts
    pushal                  # Push general-purpose registers
    push %ds                # Save data segment
    push %es
    push %fs
    push %gs

    mov $0x10, %ax          # Load kernel data segment
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    push %esp               # Pass registers_t pointer to C function
    call isr_handler        # Call C function
    add $4, %esp

    pop %gs                 # Restore segment registers
    pop %fs
    pop %es
    pop %ds
    popal                   # Restore general-purpose registers
    sti                     # Enable interrupts
    iret                    # Return from ISR

irq_stub:
    cli                     # Disable interrupts
    pushal                  # Save registers
    push %ds
    push %es
    push %fs
    push %gs

    mov $0x10, %ax          # Load kernel data segment
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    push %esp               # Pass registers_t pointer
    call irq_handler        # Call IRQ handler in C
    add $4, %esp

    pop %gs
    pop %fs
    pop %es
    pop %ds
    popal
    sti
    iret
