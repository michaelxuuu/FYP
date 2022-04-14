#ifndef PM_MNGR_H
#define PM_MNGR_H

#include "../include/type.h"

/*=======================================================================
*
*    Note1. MEMORY SIZE
*
*    The defualt memory size for i386 system emulated by QEMU is 128M Bytes
*    which we will stick to. This size is way smaller than the assumed
*    4GB, but it offers the great opportunity for us to learn to make
*    our kernel adapt to a machine with a memory size smaller than the
*    assumed 4GB, which we will discuss later when getting to paging
*    and the higher half kernel part.
*
=======================================================================*/

/**
 * 128M             Bytes
 * = 2^27           Bytes
 * = 2^24 * 2^3     Bytes
 * = 8 * 16^6       Bytes
 * = 0x8000000      Bytes
 */

#define MEM_SIZE 0x8000000

/*=======================================================================
*
*    Note2. DIVIDING MEMEORY
*
*    Because we have fixed the kernel to 512KB in size and relocated our kernel
*    from 1M to the bottom of the physical memory, to prepare the memory for
*    paging (dividing the memory into 4KB blocks), the following memory layout
*    should be yeilded at this moment, where it is fiarly noticeable that the
*    kernel area is automatically page (4KB) aligned.
*
*    +----------------------------------------+ 0x8000000 (128MB)
*    |              Block (4KB)               |
*    +----------------------------------------+ 
*                        .
*                        .
*                        .
*    +----------------------------------------+
*    |              Block (4KB)               |
*    +----------------------------------------+
*    |              Block (4KB)               |
*    +----------------------------------------+ 
*    |            Kernel (512KB)              |
*    +----------------------------------------+ 0x0
*
=======================================================================*/

/**
 * 4K           Bytes
 * = 2^12       Bytes
 * = 16^3       Bytes
 * = 0x1000     Bytes
 */

#define BLOCK_SIZE  0x1000 // 4K

#define TOTAL_BLOCK_COUNT 0x8000 // 32K

/*=======================================================================
*
*    Note3. MEMORY BITMAP
*
*    To track the status of every block - free or in use, the most efficient
*    way is to create an array of bits, in which each bit is mapped to 
*    a block of memory; thus, to denote a block as free, we just clear the 
*    bit that it is mapped to, and to mark the block in use, we just set the 
*    bit to 1. This array of bits is known as a BITMAP.
*
*    The mapping is simple: the index of a bit in the bit array is the same
*    as the index of the block in the memory: block 0 is mapped to bit 0,
*    block 1 to bit 1, nlock 2 to bit 2...
*    
*    However, there is no way to create such an array of bits. Nevertheless,
*    we can create an array of integers and use it as if it were an array of
*    integers.
*
*    How large should the integer array be? We have calculated that there are
*    32K blocks at total, and with this one-to-one mapping, there should be 
*    32K bits, which is equivolent to 1K integers, given an integer is 32 bits.
*    Therefore, we need an array of 1024 integers.
*
*    Another thing to be clear about is that the bitmap should not be 
*    allocated in the stack area but in the kernel area (in the lower
*    512 KB). By incorprating the bitmap into the kernel code/data
*    area (code and data segemnts are not separated in the Flat Memory 
*    Model), the immediate benifit we get is when we have to remap the
*    kernel to 3GB to create higher half kernel we don't need to care
*    about the kernel stack area which is now at 0x90000.
*
=======================================================================*/


/*=============================  BITMAP  ==================================*/

extern uint32_t mem_bitmap[1024]; // static uint32_t mem_bitmap[1024]; ---> Nope, otherwise two copies of mem_bitmap!!!!!!!!!

/**
 * Set a bit in the bitmap to mark the given block as in use
 * (Reminder: bit index = block index)
 */
void mem_bitmap_set(int bit_index);

/**
 * Clear a bit in the bitmap to mark the given block as free
 * (Reminder: bit index = block index)
 */
void mem_bitmap_clr(int bit_index);

/**
 * Test if the given block is free
 * Returns TRUE (any integer greater than 0) if free or 0 if not
 * (Reminder: bit index = block index)
 */
int mem_bitmap_is_free(int bit_index);

/**
 * Linear search to find the first available block
 * Returns the block index if found or 0 if not
 */
int mem_bitmap_find_free();

/*=========================  PHYSICAL MANAGER ALLOCATOR  ============================*/

/**
 * Allocate a block to use
 * Returns the physical address of the block if available frames exit or 0 if not
 */
uint32_t pm_mngr_alloc_block();

/**
 * Frees an alocated block
 */
void pm_mngr_free_block(uint32_t pa);

/**
 * Initilze the physical memory manager (mainly to mark the first 512K in use)
 */
void pm_mngr_init();


#endif