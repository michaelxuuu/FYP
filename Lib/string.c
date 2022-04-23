#include "string.h"

int str_cmp(const char * s1, const char * s2) {
    while (*s1 && *s2)
    {
        if (*s1 != *s2)
            break;

        s1++;
        s2++;
    }
    return *(uint8_t*)s1 - *(uint8_t*)s2;
}

int str_len(const char * s1) {
    int i;
    for (i = 0; s1[i]; i++);
    return i;
}