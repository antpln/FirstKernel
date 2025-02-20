#include <stdint.h>
#include "kernel/port_io.h"
#include "kernel/pic.h"
#include "kernel/isr.h"
#include "kernel/keyboard.h" // Include the new header file
#include "stdio.h"           // Include the new header file
#include "kernel/shell.h"



static bool shift_pressed = false;
static bool caps_lock_active = false;

static uint8_t scancode_to_ascii[128] = {
    0,
    0,
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    '0',
    '-',
    '=',
    0,
    0,
    'q',
    'w',
    'e',
    'r',
    't',
    'y',
    'u',
    'i',
    'o',
    'p',
    '[',
    ']',
    0,
    0,
    'a',
    's',
    'd',
    'f',
    'g',
    'h',
    'j',
    'k',
    'l',
    ';',
    '\'',
    '`',
    0,
    '\\',
    'z',
    'x',
    'c',
    'v',
    'b',
    'n',
    'm',
    ',',
    '.',
    '/',
    0,
    '*',
    0,
    ' ',
};

char toupper(char c)
{
    if (c >= 'a' && c <= 'z')
    {
        return c - ('a' - 'A');
    }
    return c;
}

char kb_to_ascii(keyboard_event event) {
    uint8_t scancode = event.scancode;
    char ascii = 0;  // Default: no character

    if (scancode < 128) {
        ascii = scancode_to_ascii[scancode];
        if (event.shift || caps_lock_active) {
            if (ascii >= 'a' && ascii <= 'z') {
                ascii = toupper(ascii);
            }
        }
    }
    return ascii;
}


keyboard_event read_keyboard() {
    keyboard_event event;
    uint8_t scancode = inb(KBD_DATA_PORT);
    char ascii = 0;  // Default: no character
    event.scancode = scancode;

    if (scancode == KBD_SCANCODE_SHIFT_LEFT || scancode == KBD_SCANCODE_SHIFT_RIGHT)
    {
        shift_pressed = true;
    }
    else if (scancode == (KBD_SCANCODE_SHIFT_LEFT | KBD_SCANCODE_RELEASE) ||
             scancode == (KBD_SCANCODE_SHIFT_RIGHT | KBD_SCANCODE_RELEASE))
    {
        shift_pressed = false;
    }
    event.shift = shift_pressed;
    event.release = scancode & KBD_SCANCODE_RELEASE;
    if (scancode == KBD_SCANCODE_CAPS_LOCK)
    {
        caps_lock_active = !caps_lock_active;
    }
    event.enter = scancode == KBD_SCANCODE_ENTER;
    event.caps_lock = caps_lock_active;
    return event;
}

void keyboard_callback(registers_t *regs)
{
    keyboard_event event = read_keyboard();
    shell_handle_key(event);
}

void wait_for_input_clear()
{
    while (inb(0x64) & 2)
    {
        // Wait until the input buffer is clear (Bit 1 == 0)
    }
}

void keyboard_flush()
{
    while (inb(0x64) & 1)
    {
        inb(0x60); // Read and discard data
    }
}



void keyboard_reset()
{
    printf("Resetting keyboard controller...\n");
    wait_for_input_clear();
    outb(0x64, 0xFF); // Send reset command

    uint8_t response = inb(0x60); // Expect 0xFA (ACK) or 0xAA (Self-test passed)
    printf("Keyboard reset response: %x\n", response);
}

void keyboard_enable()
{
    printf("Enabling keyboard...\n");
    keyboard_reset();
    keyboard_flush();

    // Wait for and acknowledge any pending commands
    wait_for_input_clear();
    outb(0x64, 0xAE); // Enable keyboard interface
    wait_for_input_clear();

    // Send keyboard Enable command
    outb(0x60, 0xF4); // Enable scanning
    
    // Wait for and read ACK
    for(int i = 0; i < 1000; i++) {
        if(inb(0x64) & 1) {  // Check if there's data to read
            uint8_t response = inb(0x60);
            printf("Keyboard response: 0x%x\n", response);
            if(response == 0xFA) { // ACK
                return;
            }
        }
    }
    printf("Warning: No ACK received from keyboard\n");
}

void keyboard_install()
{
    printf("[KB] Enabling keyboard...");
    keyboard_enable();
    register_interrupt_handler(33, keyboard_callback);
    pic_unmask_irq(1);
}

void keyboard_check_status()
{
    uint8_t status = inb(0x64); // Read status from PS/2 controller

    if (status & 0x01)
    {
        printf("Output buffer has data!\n");
    }
}

void keyboard_poll()
{
    while (1)
    {
        uint8_t status = inb(0x64);
        if (status & 0x01) read_keyboard();
    }
}

