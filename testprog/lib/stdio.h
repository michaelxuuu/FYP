#ifndef STDIO_H
#define STDIO_H

#include"stdarg.h"

#include"stdint.h"

typedef struct dirent
{

    uint8_t name[23];
    uint8_t attrib;
    uint32_t blockno;
    uint32_t size;

} __attribute__((packed)) dirent;

void putchar(char c);

void puts(const char *s);

void printf(const char *fmt, ...);

void fopen(char *path, uint32_t attrib, dirent* p);

#endif