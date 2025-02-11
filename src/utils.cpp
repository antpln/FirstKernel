#include "utils.h"

int strlen(const char* str) {
    int len = 0;
    while (str[len]) {
        len++;
    }
    return len;
}
void int_to_string(int num, char* buffer) {
    int i = 0;
    int is_negative = 0;
    if (num < 0) {
        is_negative = 1;
        num = -num;
    }
    while (num != 0) {
        buffer[i++] = num % 10 + '0';
        num /= 10;
    }
    if (is_negative) {
        buffer[i++] = '-';
    }
    buffer[i] = '\0';
    int len = strlen(buffer);
    for (int j = 0; j < len / 2; j++) {
        char temp = buffer[j];
        buffer[j] = buffer[len - j - 1];
        buffer[len - j - 1] = temp;
    }
}