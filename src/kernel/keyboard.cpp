#include "keyboard.h"
#include "kernel/isr.h"
#include "kernel/port_io.h"
#include "kernel/debug.h"
#include <stdio.h>

#define KEYBOARD_BUFFER_SIZE 256

static char keyboard_buffer[KEYBOARD_BUFFER_SIZE];
static uint8_t buffer_head = 0;
static uint8_t buffer_tail = 0;

/* Keyboard Scancode Set (Basic US QWERTY) */
static char scancode_to_ascii[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t', 
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0, /* Left control */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, /* Left shift */
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, /* Right shift */
    '*', 0, /* Left Alt */ ' ', /* Space bar */ 0, /* Caps lock */};

/* Circular buffer push */
static void keyboard_buffer_push(char c) {
    uint8_t next = (buffer_head + 1) % KEYBOARD_BUFFER_SIZE;
    if (next != buffer_tail) {
        keyboard_buffer[buffer_head] = c;
        buffer_head = next;
    }
}

/* Circular buffer pop */
char keyboard_getchar() {
    if (buffer_head == buffer_tail) {
        return 0; 
    }
    char c = keyboard_buffer[buffer_tail];
    buffer_tail = (buffer_tail + 1) % KEYBOARD_BUFFER_SIZE;
    return c;
}

/* Keyboard Interrupt Handler */
void keyboard_handler() {
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);

    if (scancode < 128) { // Ignore key releases for now
        char c = scancode_to_ascii[scancode];
        if (c) {
            keyboard_buffer_push(c);
        }
    }
}

void keyboard_init() {
    printf("[DEBUG] Entering keyboard_init()\n");

    outb(0x64, 0xAE);  

    register_interrupt_handler(33, keyboard_handler);

    printf("[INIT] Keyboard initialized.\n");
}