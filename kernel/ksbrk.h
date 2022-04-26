#ifndef KSBRK_H
#define KSBRk_H

#include"../include/type.h"

#include"../mem/vm_mngr.h"

#include"../mem/pm_mngr.h"

#define KERNEL_HEAP_BASE 0xC0101000

/**
 * This function grows or shrinks the kernel heap
 * When it grows the kernel heap, new PHYSICAL frames get allocated
 * and when it shrinks the heap, in addiation to pages get unmapped, PHYSICAL frames also get freed
 * 
 * Takes the argument of the kernel heap increment amount (decrement if negative)
 * Returns the previous break address
 */
uint32_t ksbrk(int inc);

#endif