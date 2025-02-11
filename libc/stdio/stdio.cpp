#include <stdio.h>
#include <kernel/vga.h>
#include <stdarg.h>

extern Terminal terminal; // Add this line to access the singleton defined in kernel.cpp

int putchar(char c) {
    terminal.putchar(c);
    return c;
}

int puts(const char* str) {
    terminal.writestring(str);
    terminal.writestring("\n");
    return 0;
}

// Convert integer to string (helper function for printf)
static void itoa(int value, char* buffer, int base) {
    static char digits[] = "0123456789ABCDEF";
    char temp[32];
    int i = 0, j = 0;
    
    if (value < 0 && base == 10) {
        buffer[j++] = '-';
        value = -value;
    }

    do {
        temp[i++] = digits[value % base];
        value /= base;
    } while (value);

    while (i > 0) {
        buffer[j++] = temp[--i];
    }
    buffer[j] = '\0';
}

// Simple `printf` implementation
int printf(const char* format, ...) {
    va_list args;
    va_start(args, format);

    char buffer[32];

    while (*format) {
        if (*format == '%') {
            format++;
            switch (*format) {
                case 'd': { // Integer
                    int num = va_arg(args, int);
                    itoa(num, buffer, 10);
                    terminal.writestring(buffer);
                    break;
                }
                case 'x': { // Hexadecimal
                    int num = va_arg(args, int);
                    itoa(num, buffer, 16);
                    terminal.writestring(buffer);
                    break;
                }
                case 's': { // String
                    char* str = va_arg(args, char*);
                    terminal.writestring(str);
                    break;
                }
                case 'c': { // Character
                    char c = (char)va_arg(args, int);
                    terminal.putchar(c);
                    break;
                }
                case '%': { // Literal %
                    terminal.putchar('%');
                    break;
                }
                default:
                    terminal.putchar('%');
                    terminal.putchar(*format);
            }
        } else {
            terminal.putchar(*format);
        }
        format++;
    }

    va_end(args);
    return 0;
}
