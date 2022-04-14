#ifndef HEAP_H
#define HEAP_H

#include"../include/type.h"

#include"ksbrk.h"

#include"../mem/pm_mngr.h"

#include"../mem/vm_mngr.h"

void* kmalloc(uint32_t size);

void kfree(void *ptr);

#endif