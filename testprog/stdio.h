#ifndef LIBC_H
#define LIBC_H

#define PRINT(s) \
    __asm__ volatile \
    ( \
        "mov $0x0, %%eax;" \
        "mov %0, %%edi;" \
        "int $0x80;"\
        :: "b"((int)s)\
    );

#define FOPEN(name, attrib, dirent_ptr) \
    __asm__ volatile \
    ( \
        "mov $0x1, %%eax;" \
        "mov %0, %%edi;" \
        "mov %1, %%esi;" \
        "mov %2, %%edx;" \
        "int $0x80;" \
        :: "r"((int)name), "r"((int)attrib), "r"((int)dirent_ptr) \
    );

#endif