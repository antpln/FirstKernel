#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stddef.h>
size_t strlen(const char* str);
void int_to_string(int num, char* buffer);
uint16_t low_16(uint32_t addr);
uint16_t high_16(uint32_t addr);

#endif