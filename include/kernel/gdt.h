#include <stdint.h>

// Each GDT entry is 8 bytes.
struct __attribute__((packed)) GDTEntry {
    uint16_t limit_low;     // lower 16 bits of limit
    uint16_t base_low;      // lower 16 bits of base
    uint8_t  base_mid;      // next 8 bits of base
    uint8_t  access;        // access flags
    uint8_t  granularity;   // granularity + top 4 bits of limit
    uint8_t  base_high;     // final 8 bits of base
};

// The GDTPtr is used by the 'lgdt' instruction. 
// It tells the CPU where our GDT is and its size.
struct __attribute__((packed)) GDTPtr {
    uint16_t limit;
    uint32_t base;
};

void init_gdt();