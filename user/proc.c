#include"proc.h"

typedef struct proc proc;

struct proc
{
    int id;
    uint32_t esp, ebp, eip;
    uint32_t *pd;
     
    proc *next;
    proc *prev;
};

extern uint32_t *cur_pd;        // defined in vm_mngr.c

extern uint32_t *kernel_pd;     // defined in vm_mngr.c

typedef struct queue
{
    proc *cur;
    proc *head;
}queue;

queue *readyqueue;

int nextid = 0;

void multiprocing_init()
{
    readyqueue = (queue*)kmalloc(sizeof(queue));
    // Create first proc which is the kernel proc
    readyqueue->head->id = nextid++;
    readyqueue->head = (proc*)kmalloc(sizeof(proc));
    readyqueue->head->esp = 0; // regs initialized to 0s and will be assigned with valid values during swicthing (leaving current proc) 
    readyqueue->head->ebp = 0;
    readyqueue->head->eip = 0;
    readyqueue->head->pd = cur_pd;
    readyqueue->head->next = readyqueue->head;
    readyqueue->head->prev = readyqueue->head;
}

void copy_pt(uint32_t *src, uint32_t *dest)
{
    // Create a page diretcory
    dest = (uint32_t*)kmalloc(4096);

    // copy the page directory
    for (int i = 0; i < 1024; i++)
        dest[i] = src[i];

    // Copy each page table if allocated
    for (uint32_t *pde = 0; pde < dest + 1024; pde++)
    {
        if (!page_is_present(*pde))
            continue;

        // Create a page table
        uint32_t dest_pt_physical_addr = pm_mngr_alloc_block();
        // Link the page table to the page directory
        page_install_frame_addr(pde, dest_pt_physical_addr);

        // Get src page table physical address
        uint32_t src_pt_physical_addr = page_get_frame_addr(*pde);

        // Now we have the physical addresses of the dest and src page table but neither of them is mapped so we are currently unable to access them
        // To access them, we need to first map them to the virtual address space with the help of the mapped utility page
        // Pointer to the utility page
        uint32_t *util_pt = (uint32_t*)0xC0100000;
        // Map the src pt to 0xFFC00000 so that we can modify it through writing to the virtual address from 0xFFC00000 to 0xFFC00fff (4K)
        page_install_frame_addr(util_pt, src_pt_physical_addr);
        page_add_attrib(util_pt, PAGE_PRESENT | PAGE_WRITABLE);
        // Map the src pt to 0xFFC01000 so that we can modify it through writing to the virtual address from 0xFFC01000 to 0xFFC01fff (4K)
        page_install_frame_addr(util_pt, dest_pt_physical_addr);
        page_add_attrib(util_pt, PAGE_PRESENT | PAGE_WRITABLE);

        // Pointer to the src page table
        uint32_t *src_pt = (uint32_t*)0xFFC00000;

        // Pointer to the dest page table
        uint32_t *dest_pt = (uint32_t*)0xFFC01000;
        
        // Copy!
        for (int i = 0; i < 1024; i++)
            dest_pt[i] = src_pt[i];

        // Clear the utility page (umapping the page tables)
        *util_pt = 0;
        *(util_pt + 1) = 0;
    }
}

void destory_py(uint32_t *pt)
{
    for (uint32_t *pde; pde < pt + 1024; pde++)
        pm_mngr_free_block(page_get_frame_addr(*pde));
    kfree(pt);
}

proc *cur_proc;

int create_proc()
{
    proc *paren = cur_proc;

    uint32_t *child_pt;

    copy_pt(paren->pd, child_pt);

    // Create child process
    proc *child = (proc*)kmalloc(sizeof(proc));

    child->id  = nextid++;
    child->esp = 0; // regs initialized to 0s and will be assigned with valid values during swicthing (leaving current proc) 
    child->ebp = 0;
    child->eip = 0;
    child->pd = child_pt;

    // head->prev <=> child <=> head
    readyqueue->head->prev->next = child;
    child->prev = readyqueue->head->prev;
    child->next = readyqueue->head;
    readyqueue->head->prev = child;

    return child->id;
}


