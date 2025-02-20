#include <string.h>

char* strtok(char* str, const char* delim) {
    static char* last = NULL;
    if (str != NULL) {
        last = str;
    } else {
        if (last == NULL) {
            return NULL;
        }
    }

    // Skip leading delimiters.
    while (*last != '\0' && strchr(delim, *last) != NULL) {
        last++;
    }

    if (*last == '\0') {
        return NULL;
    }

    char* start = last;
    // Find the end of the token.
    while (*last != '\0' && strchr(delim, *last) == NULL) {
        last++;
    }

    if (*last != '\0') {
        *last = '\0';
        last++;
    }

    return start;
}