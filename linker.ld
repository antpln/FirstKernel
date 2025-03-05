/* The bootloader starts execution at _start (defined in your assembly or C). */
ENTRY(_start)

SECTIONS
{
    /* Link/load addresses start at 2 MiB (0x00200000). */
    . = 0x00200000;

    /* 
     * The .text section, with the multiboot header first,
     * so the bootloader sees the magic immediately.
     */
    .text BLOCK(4K) : ALIGN(4K)
    {
        /* Place the multiboot header at the very start */
        *(.multiboot)

        /* Then the normal .text code */
        *(.text)
    }

    /* Example: place .lowmem after .text. You may choose to put it first if you want. */
    .lowmem BLOCK(4K) : ALIGN(4096)
    {
        *(.lowmem)
    }

    /* Read-only data. */
    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata)
    }

    /* Initialized data. */
    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data)
    }

    /* Uninitialized data (BSS). */
    .bss BLOCK(4K) : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
    }

    /* Bootstrap stack. */
    .bootstrap_stack ALIGN(16) :
    {
        __stack_bottom = .;
        *(.bootstrap_stack)
        __stack_top = .;
    }

    /* Mark the end of the kernel image. */
    . = ALIGN(4);
    kernel_end = .;
}
