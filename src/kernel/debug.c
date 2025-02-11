#include <kernel/debug.h>
#include <stdio.h>

#define DEBUG 1
#define TEST 1
void panic(const char* message) {
    printf("[PANIC] %s\n", message);
    while (1) {
        __asm__("cli; hlt");  // Halt CPU to prevent further execution
    }
}
void debug(const char* message) {
    printf("[DEBUG] %s\n", message);
}
void success(const char* message) {
    printf("[SUCCESS] %s\n", message);
}
void error(const char* message) {
    printf("[ERROR] %s\n", message);
}
void test(const char* message) {
    printf("[TEST] %s\n", message);
}