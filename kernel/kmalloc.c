#include"kmalloc.h"

extern uint32_t break_addr; // defined in ksbrk.c

typedef struct hole_header hole;

struct hole_header
{
    int     free;
    int     size;
    hole    *next;
};

typedef struct hole_list
{
    hole *last;
    hole first;
} hole_list;

#define HEAP_HEADER_SIZE sizeof(struct hole_list)
#define HOLE_HEADER_SIZE sizeof(struct hole_header)

void* kmalloc(uint32_t size)
{
    if (!size)  // return NULL if size equals 0
        return 0;

    hole_list *l = 0; // Pointer to the kernel heap

    hole *h = 0; // Will point to the first hole that fits

    uint32_t size_left = 0; // If a hole is only to be taken up partilly, then the rest of it would form a new hole, and this variable holds the size of that induced hole

    if (break_addr == KERNEL_HEAP_BASE) // Firs allocation
    {
        l = (hole_list*)ksbrk(size);
        h = &(l->first);
        h->free = 0;
        h->next = 0;
        h->size = size;

        l->last = h;

        size_left = (break_addr - (uint32_t)h) - size - HEAP_HEADER_SIZE;
    }
    else // Linear search for the first fit
    {
        l = (hole_list*)KERNEL_HEAP_BASE;
        
        for (h = &(l->first); h != 0; h = h->next)
            if (h->free && h->size >= size)
                break;
        
        // Not found
        if (!h)
        {
            h = (hole*)ksbrk(size);
            h->free = 0;
            h->next = 0;
            h->size = size;

            l->last->next = h;
            l->last = h;

            size_left = (break_addr - (uint32_t)h) - size - HOLE_HEADER_SIZE;
        }
        else // Found
        {
            h->free = 0;
            size_left = h->size - size; // Update hole size
            h->size = size;
        }
    }

    // If the size left in the hole is sufficient for the creation of another hole, we create a new hole to take over the space left
    if (size_left > HOLE_HEADER_SIZE)
    {
        hole *new_hole = (hole*)((uint32_t)h + HOLE_HEADER_SIZE + size);
        new_hole->free = 1;
        new_hole->size = size_left - HOLE_HEADER_SIZE;
        
        // Insert 'new_hole' between 'h' and 'h->next'
        hole *temp = h->next;
        h->next = new_hole;
        new_hole->next = temp;

        // If 'h' is the last hole, we update the last hole to 'new_hole'
        if (l->last == h)
            l->last = new_hole;
    }

    return (void *)((uint32_t)h + HOLE_HEADER_SIZE);
}

void kfree(void* ptr)
{
    if (!ptr)
        return;

    hole_list *l = (hole_list*)KERNEL_HEAP_BASE;

    hole *h = (hole*)((uint32_t)ptr - HOLE_HEADER_SIZE); // ptr is not pointing to the header but the data!!! must convert it manually!!!
    
    hole *temp_h = h->next;

    h->free = 1;

    // Merge contigous free chunks
    for (; temp_h != 0 && temp_h->free; temp_h = temp_h->next)
        h->size += (temp_h->size + HOLE_HEADER_SIZE);
    
    if(temp_h != h->next)
        h->next = temp_h;
    
    if(!temp_h)
        l->last = h;
}