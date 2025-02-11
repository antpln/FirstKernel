#ifndef SERIAL_H
#define SERIAL_H

void serial_init();
void serial_write(char c);
void serial_print(const char* str);
char serial_read();
void serial_readline(char* buffer, int size);
#endif
