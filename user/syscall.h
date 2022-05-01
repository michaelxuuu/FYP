#ifndef SYSCALL_H
#define SYSCALL_H

#include"../include/type.h"

#include"../kernel/kprintf.h"

#include"../drivers/screen.h"

#include"../drivers/hd.h"

#include"../cpu/comn_hdlr.h"

#include"../cpu/idt.h"

#include"../fs/fs.h"

#include"../user/proc.h"

#include"../kernel/ksbrk.h"

#include"../drivers/screen.h"

#define SYSCALL_FUNC void(*)(int, int, int, int, int, int)

#define SYSCALL0(name) void syscall_##name(int edi, int esi, int edx, int ecx, int ebx, int r)
#define SYSCALL1(name, p1) void syscall_##name(int p1, int esi, int edx, int ecx, int ebx, int r)
#define SYSCALL2(name, p1, p2) void syscall_##name(int p1, int p2, int edx, int ecx, int ebx, int r)
#define SYSCALL3(name, p1, p2, p3) void syscall_##name(int p1, int p2, int p3, int ecx, int ebx, int r)
#define SYSCALL4(name, p1, p2, p3, p4) void syscall_##name(int p1, int p2, int p3, int p4, int ebx, int r)
#define SYSCALL5(name, p1, p2, p3, p4, p5) void syscall_##name(int p1, int p2, int p3, int p4, int p5, int r)

void syscall_init();

void register_syscall(int syscallno, void *func);

SYSCALL0(exit);

#endif