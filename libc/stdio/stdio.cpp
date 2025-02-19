#include <stdio.h>
#include <kernel/vga.h>
#include <stdarg.h>

extern Terminal terminal; // Access singleton terminal object

int putchar(char c) {
    terminal.putchar(c);
    return c;
}

int puts(const char* str) {
    terminal.writestring(str);
    terminal.writestring("\n");
    return 0;
}

// Convert integer to string (supports base 10 and 16)
static void itoa(uint32_t value, char* buffer, int base) {
    static char digits[] = "0123456789ABCDEF";
    char temp[32]; // Temporary buffer
    int i = 0;

    if (value == 0) {  // Handle 0 explicitly
        buffer[i++] = '0';
    } else {
        while (value) { 
            temp[i++] = digits[value % base];
            value /= base;
        }
    }

    // Reverse the string into `buffer`
    int j = 0;
    while (i > 0) {
        buffer[j++] = temp[--i]; 
    }
    buffer[j] = '\0';  // Null-terminate the string
}

int printf(const char* format, ...) {
    va_list args;
    va_start(args, format);

    char buffer[32];

    while (*format) {
        if (*format == '%') {
            format++;
            switch (*format) {
                case 'd': { // Signed Integer
                    int num = va_arg(args, int);
                    if (num < 0) {
                        terminal.putchar('-'); // Print minus sign for negative numbers
                        num = -num;
                    }
                    itoa(num, buffer, 10);
                    terminal.writestring(buffer);
                    break;
                }
                case 'u': { // Unsigned Integer
                    unsigned int num = va_arg(args, unsigned int);
                    itoa(num, buffer, 10);
                    terminal.writestring(buffer);
                    break;
                }
                case 'x': { // Hexadecimal
                    unsigned int num = va_arg(args, unsigned int);
                    terminal.writestring("0x"); // Add '0x' prefix for clarity
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
                case 'p': { // Pointer
                    void* ptr = va_arg(args, void*);
                    terminal.writestring("0x");
                    itoa((uintptr_t)ptr, buffer, 16);
                    terminal.writestring(buffer);
                    break;
                }
                case '%': { // Literal '%'
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
