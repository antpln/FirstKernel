#ifndef PIC_H
#define PIC_H

#include <stdint.h>

void pic_remap();
void pic_send_eoi(uint8_t irq);
void pic_unmask_irq(uint8_t irq);
void init_pic();
#endif // PIC_H
