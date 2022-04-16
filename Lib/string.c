#include "string.h"

int str_cmp(const char * s1, const char * s2, int size) {
    for (int i = 0; i < size; i++)
    {
        if (s1[i] > s2[i])
            return 1;
        else if (s1[i] < s2[i])
            return -1;
    }
    return 0;
}

int str_len(const char * s1) {
    int i;
    for (i = 0; s1[i]; i++);
    return i;
}