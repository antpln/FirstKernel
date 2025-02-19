#include <kernel/idt.h>
#include <stdio.h>

#define IDT_ENTRIES 256

static IDTEntry idt[IDT_ENTRIES];
static IDTDescriptor idt_desc;

extern "C" void load_idt(uint32_t);

extern "C" void* isr_stub_table[32];
extern "C" void* irq_stub_table[16];

void debug_idt_entry(int i) {
    uint32_t base = (idt[i].offset_high << 16) | (idt[i].offset_low);
    printf("IDT[%d]: base=0x%x selector=0x%x flags=0x%x\n",
           i, base, idt[i].selector, idt[i].type_attr);
}

void init_idt() {
    idt_desc.limit = (sizeof(IDTEntry) * IDT_ENTRIES) - 1;
    idt_desc.base = (uint32_t)&idt;

    printf("[IDT] Initializing IDT base at 0x%x and limit to %x\n", idt_desc.base, idt_desc.limit);

    // Clear IDT
    for (uint16_t i = 0; i < IDT_ENTRIES; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    // Set CPU exceptions (0-31) to their respective handlers
    for (uint8_t i = 0; i < 32; i++) {
        idt_set_gate(i, (uint32_t)isr_stub_table[i], 0x08, 0x8E);  // Interrupt Gate
    }

    // Set IRQs (32-47) to their respective handlers
    for (uint8_t i = 32; i < 48; i++) {
        idt_set_gate(i, (uint32_t)irq_stub_table[i - 32], 0x08, 0x8E);  // Interrupt Gate
        //printf("[IDT] Setting IRQ %d -> handler at 0x%x\n", i - 32, (uint32_t)irq_stub_table[i - 32]);
    }
    // Load IDT
    load_idt((uint32_t)&idt_desc);
}




void idt_set_gate(uint8_t num, uint32_t offset, uint16_t selector, uint8_t flags) {
    idt[num].offset_low = offset & 0xFFFF;
    idt[num].offset_high = (offset >> 16) & 0xFFFF;
    idt[num].selector = selector;
    idt[num].zero = 0;
    idt[num].type_attr = flags | 0x80; // Ensure Present (P) bit is set

    //printf("[IDT] Setting gate %d -> handler at 0x%x\n", num, offset);
}

