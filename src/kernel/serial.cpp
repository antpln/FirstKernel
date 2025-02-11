#include <stdint.h>
#include <kernel/serial.h>
#include <kernel/port_io.h>

// Define the COM1 port address
#define COM1 0x3F8  // Base I/O address for COM1
#define COM2 0x2F8  // Base I/O address for COM2
#define COM3 0x3E8  // Base I/O address for COM3
#define COM4 0x2E8  // Base I/O address for COM4


// Initialize COM1 serial port
void serial_init() {
    outb(COM1 + 1, 0x00);    // Disable interrupts
    outb(COM1 + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(COM1 + 0, 0x03);    // Set divisor to 3 (38400 baud)
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(COM1 + 2, 0xC7);    // Enable FIFO, clear them, 14-byte threshold
    outb(COM1 + 4, 0x0B);    // Enable IRQs, RTS/DSR set
}

// Check if the serial port is ready to send data
static int serial_is_transmit_ready() {
    return inb(COM1 + 5) & 0x20;  // Bit 5 of line status register
}

// Send a character over the serial port
void serial_write(char c) {
    while (!serial_is_transmit_ready());  // Wait until ready
    outb(COM1, c);
}

// Send a null-terminated string over the serial port
void serial_print(const char* str) {
    while (*str) {
        serial_write(*str++);
    }
}
