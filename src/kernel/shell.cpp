
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <kernel/keyboard.h>
#include <kernel/isr.h>

#define SHELL_BUFFER_SIZE 256

// A global command buffer and index to keep track of the current command.
static char shell_buffer[SHELL_BUFFER_SIZE];
static uint32_t shell_index = 0;

// Process a completed command line.
void shell_process_command(const char* cmd) {
    // For this basic shell, we'll support three commands:
    //  1. "help"  : Print available commands.
    //  2. "clear" : Clear the screen.
    //  3. "echo ..." : Echo back the provided text.
    if (strcmp(cmd, "help") == 0) {
        printf("Available commands: help, clear, echo <text>\n");
    }
    else if (strcmp(cmd, "clear") == 0) {
        // Simple screen clear by printing several newlines.
        for (int i = 0; i < 50; i++) {
            printf("\n");
        }
    }
    else if (strncmp(cmd, "echo ", 5) == 0) {
        printf("%s\n", cmd + 5);
    }
    else if (strlen(cmd) == 0) {
        // Empty command, do nothing.
    }
    else {
        printf("Unknown command: %s\n", cmd);
    }
}

// This function is called by the keyboard interrupt handler whenever a valid character arrives.
void shell_handle_key(keyboard_event ke) {
    // Handle backspace. We assume backspace is sent as ASCII 0x08 or DEL (127).
    if (ke.scancode == '\b' || ke.scancode == 127) {
        if (shell_index > 0) {
            shell_index--;
            // Erase the last character on screen (move back, print space, move back again).
            printf("\b \b");
        }
        return;
    }

    // If the character is newline or carriage return, we process the command.
    if (ke.scancode == '\n' || ke.scancode == '\r' || ke.enter) {
        // Terminate the command string.
        shell_buffer[shell_index] = '\0';
        printf("\n");
        shell_process_command(shell_buffer);
        // Reset for the next command.
        shell_index = 0;
        printf("nutshell> ");
        return;
    }

    if(ke.backspace) {
        if (shell_index > 0) {
            shell_index--;
            // Erase the last character on screen (move back, print space, move back again).
            printf("\b \b");
        }
        return;
    }

    // If the character is a printable character, we add it to the buffer.
    char c = kb_to_ascii(ke);
    if(c != 0) {
        // Check if we have space in the buffer.
        if (shell_index < SHELL_BUFFER_SIZE - 1) {
            shell_buffer[shell_index] = c;
            shell_index++;
            printf("%c", c);
        }
    }
}

// Initialize the shell (print a welcome message and the prompt).
void shell_init() {
    printf("Welcome to nutshell!\n");
    printf("nutshell> ");
}
