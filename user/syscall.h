#ifndef SYSCALL_H
#define SYSCALL_H

#include"../include/type.h"

#include"../kernel/kprintf.h"

#include"../drivers/screen.h"

#include"../drivers/hd.h"

#include"../cpu/comn_hdlr.h"

#include"../cpu/idt.h"

void syscall_init();

void register_syscall(int syscallno, void *func);

#endif