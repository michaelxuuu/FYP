#ifndef PROC_H
#define PROC_H

#include"../include/type.h"

#include"../kernel/kmalloc.h"

#include"../mem/vm_mngr.h"

#include"../mem/pm_mngr.h"

void multitasking_init();

void switch_task();

int fork();

int getpid();

#endif