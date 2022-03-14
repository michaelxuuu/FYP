#include"pm_mngr.h"

/**
 * Given the index of a bit, the index of the ineteger 
 * in which the bit resides is index / 32, and the bit 
 * offset of that bit in the integer is index % 32
 */

uint32_t mem_bitmap[1024];

void
mem_bitmap_set(int bit_index) {
    mem_bitmap[bit_index / 32] |= 1 << bit_index % 32;
}

void
mem_bitmap_clr(int bit_index) {
    mem_bitmap[bit_index / 32] &= ~(1 << bit_index % 32);
}

int
mem_bitmap_is_free(int bit_index) {
    return mem_bitmap[bit_index / 32] & (1 << bit_index % 32);
}

int 
mem_bitmap_find_free() {
    // Loop through all 1024 integers
    for (int i = 0; i < 1024; i++)
        // To optimize, we can first test if the dward is all set, 
        // and if yes there is no need for iteraing each individual bit
        if (mem_bitmap[i] != 0xFFFFFFFF)
            // Loop through all 32 possible bit offset in each integer
            for (int offset = 0; offset < 32; offset++)
                // Found a 0, then return
                if ( !(mem_bitmap[i] & (1 << offset)) )
                    return i * 32 + offset;
    return -1;
}

physical_addr
pm_mngr_alloc_block() {

    // Find in the bitmap the bit that corresponds to the first available block (frame)
    int bit_index = mem_bitmap_find_free();

    // Out of memory
    if (bit_index == -1)
        return 0;

    // Set the bit in the bitmap to mark the block as in use
    mem_bitmap_set(bit_index);

    // Calculate the physical address using this bit number
    uint32_t addr = bit_index * BLOCK_SIZE;

    // Return the physical address of the block
    return addr;
}

void
pm_mngr_free_block(physical_addr pa) {

    // Calculate the bit
    int bit_index = pa / BLOCK_SIZE;

    // Clear the bit in the bitmap to mark the block as free
    mem_bitmap_clr(bit_index);
}

void 
pm_mngr_init() {

    // Mark first 512KB as in use
    // 512KB is 512/4 = 128 blocks which is 128/32 = 4 integers
    int i = 0;
    for (; i < 4; i++)
        mem_bitmap[i] = 0xFFFFFFFF;

    // Mark all the memory higher than 512KB as free
    for (; i < 1024; i++)
        mem_bitmap[i] = 0x0;
}

