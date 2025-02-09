#include <kernel/idt.h>

#define IDT_ENTRIES 256

static IDTEntry idt[IDT_ENTRIES];
static IDTDescriptor idt_desc;

extern "C" void load_idt(uint32_t);

void init_idt() {
    idt_desc.limit = (sizeof(IDTEntry) * IDT_ENTRIES) - 1;
    idt_desc.base = (uint32_t)&idt;

    // Clear IDT
    for(uint16_t i = 0; i < IDT_ENTRIES; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    // Load IDT
    load_idt((uint32_t)&idt_desc);
}

void idt_set_gate(uint8_t num, uint32_t offset, uint16_t selector, uint8_t flags) {
    idt[num].offset_low = offset & 0xFFFF;
    idt[num].offset_high = (offset >> 16) & 0xFFFF;
    idt[num].selector = selector;
    idt[num].zero = 0;
    idt[num].type_attr = flags;
}