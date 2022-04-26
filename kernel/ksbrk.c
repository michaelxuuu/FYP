#include"ksbrk.h"

uint32_t break_addr = KERNEL_HEAP_BASE;

/**
 * 
 * +---------------+ <------- BREAK
 * |               |
 * +---------------+
 * |               |
 * +---------------+
 * |               |
 * +---------------+ <------- 0xC0102000 (HEAP BASE)
 * 
 * BREAK always falls into the first page (which is unmapped) above the heap top; 
 * so, to free, start from (break - 1) and to map start from BREAK
 * 
 */

uint32_t ksbrk(int inc)
{
    uint32_t prior_break = break_addr;

    uint32_t block_ct = 0; // number of blocks to allocate or free

    if (inc > 0)
        block_ct = inc % 4096 > 0 ? inc / 4096 + 1 : inc / 4096; // Round up if grow
    else
        block_ct = -inc / 4096; // Round down if shrink

    for (int i = 0; i < block_ct; i++, break_addr = inc > 0 ? break_addr + 4096 : break_addr - 4096)
        if (inc > 0)
            vm_mngr_higher_kernel_map(KERNEL_PD_BASE, break_addr, pm_mngr_alloc_block(), PAGE_PRESENT | PAGE_WRITABLE);
        else if (break_addr == KERNEL_HEAP_BASE) // Stop freeing heap pages when hitting the heap base
                return KERNEL_HEAP_BASE;
        else 
            vm_mngr_higher_kernel_unmap(KERNEL_PD_BASE, break_addr - 1);

    return prior_break;
}