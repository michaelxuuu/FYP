#include"proc.h"

procqueue *pq;

// Link proc to the end of queue of proc's (fifo)
void link_proc(proc *p)
{
    if (!pq->proc_ct)
    {   // first proc
        pq->head = p->next = p->prev = p;
        return;
    }

    // head->prev <=> p <=> head

    // head->prev <=> p
    pq->head->prev->next = p;
    p->prev = pq->head->prev;

    // p <=> head
    p->next = pq->head;
    pq->head->prev = p;
}

uint32_t nextid = 0;

void proc_set_context(proc *p)
{
    p->con.esp = 0xC0000000;
    p->con.ebp = 0xC0000000;
    p->con.eip = 0x0;
    p->con.edi = 0;
    p->con.esi = 0;
    p->con.eax = 0;
    p->con.ebx = 0;
    p->con.ecx = 0;
    p->con.edx = 0;
}

void proc_init()
{
    __asm__ volatile ("cli;");
    // create a proc queue
    pq = (procqueue*)kmalloc(sizeof(procqueue));
    pq->cur = pq->head = 0;
    pq->proc_ct = 0;

    // create proc address space which will be passed to the first proc
    // proc text
    for (int i = 0; i < 4; i++)
        vm_mngr_higher_kernel_map(i * 4096, pm_mngr_alloc_block(), PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER);
    // proc heap
    vm_mngr_higher_kernel_map(0x4000, pm_mngr_alloc_block(), PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER);
    // proc stack
    vm_mngr_higher_kernel_map(0xC0000000 - 0x1000, pm_mngr_alloc_block(), PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER);


    // extern uint8_t firstproc_start;
    // extern uint8_t firstproc_end;
    
    // wirte executable binaries into the page which will then be executed
    // __asm__ volatile 
    // (
    //     "jmp firstproc_end;"
    //     "firstproc_start:;"
    //     ".global firstproc_start;"
    //     ".string \"syscall0\";"
    //     "mov $0, %eax;"
    //     "mov $0, %edi;"
    //     "int $128;"       // test user interrupt!!!
    //     "firstproc_end:;"
    //     ".global firstproc_end;"
    // );
    
    // mem_copy(&firstproc_start, (uint8_t*)0x0, (&firstproc_end) - (&firstproc_start));

    // first proc
    cur_proc = create_proc();

    // execute first proc
    goto_user();
}

uint32_t get_pd_physical_addr(proc *p)
{
    // get pgae table physical addr
    uint32_t pt_phy_addr = page_get_frame_addr(kernel_pd[va_get_dir_index((uint32_t)p->pd)]);

    // map the page table address through utility page
    uint32_t *util_pt = (uint32_t*)0xC0100000;
    // Map the src pt to 0xFFC00000 so that we can modify it through writing to the virtual address from 0xFFC00000 to 0xFFC00fff (4K)
    page_install_frame_addr(util_pt, pt_phy_addr);
    page_add_attrib(util_pt, PAGE_PRESENT | PAGE_WRITABLE);

    // Pointer to the page table
    uint32_t *pt = (uint32_t*)0xFFC00000;

    // get frame's addr (physical)
    uint32_t frame_addr = page_get_frame_addr(pt[va_get_page_index((uint32_t)p->pd)]);

    __asm__ volatile ("invlpg (%0);" :: "r"(0xFFC00000));

    // Plus the offset is the physical address of the page directory
    return frame_addr + va_get_page_offset((uint32_t)p->pd);

}

// pt is inherited from the parent proc... which is the currently running proc
void proc_alloc_pt(proc *child)
{
    uint32_t *src = cur_proc->pd;
    if (nextid == 0);
        src = cur_pd;
    uint32_t *dest = child->pd;

    // Create a page diretcory
    dest = (uint32_t*)kmalloc(4096);

    // copy the page directory
    for (int i = 0; i < 1024; i++)
        dest[i] = src[i];

    // Copy each page table if allocated
    for (uint32_t *pde = dest; pde < dest + 1024; pde++)
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
        page_install_frame_addr(util_pt + 1, dest_pt_physical_addr);
        page_add_attrib(util_pt + 1, PAGE_PRESENT | PAGE_WRITABLE);

        // Pointer to the src page table
        uint32_t *src_pt = (uint32_t*)0xFFC00000;

        // Pointer to the dest page table
        uint32_t *dest_pt = (uint32_t*)0xFFC01000;

        // Copy!
        // No previlige changing needed here since we are copying only the pages above 0xC0000000 which is the kernel space!
        for (int i = 0; i < 1024; i++)
            dest_pt[i] = src_pt[i];

        // Clear the utility page (umapping the page tables)
        *util_pt = 0;
        *(util_pt + 1) = 0;

        // flush tlb
        __asm__ volatile ("invlpg (%0);" :: "r"(0xFFC00000));
        __asm__ volatile ("invlpg (%0);" :: "r"(0xFFC01000));
    }
}

void proc_destory_pt(proc *p)
{
    for (uint32_t *pde; pde < p->pd + 1024; pde++)
        pm_mngr_free_block(page_get_frame_addr(*pde));
    kfree(p->pd);
}

void proc_assign_id(proc *p)
{
    p->id = nextid++;
}

void proc_assign_paren(proc *p)
{
    p->parent = cur_proc;
}

void proc_assign_dir(proc *p)
{
    p->wdir = p->parent->wdir;
}

proc *cur_proc = 0; // pointer to the currently running process

extern dirent sys_root_dir; // defined in fs.c

void proc_load_text(proc *p, char* path)
{
    dirent *e = fs_find(path, DIRENT_ATTRIB_USED);
    if (!e)
        return;
    
    for (uint32_t blockno = e->blockno, add_to_wirte = 0x0; blockno != 0; blockno = fat_get_next(blockno), add_to_wirte += 4096)
    {
        buf *b = bread(blockno);
        mem_copy(b->data, (char*)add_to_wirte, 4096);
        brelease(b);
    }
    kfree(e);
}

void proc_buf_init(proc *p)
{
    mem_set(p->inbuf.data, 0, 64);
    p->inbuf.index = 0;
    p->inbuf.ct = 0;
}

int proc_buf_read(proc *p)
{
    if (p->inbuf.ct == 0)
        return -1;

    int data = p->inbuf.data[p->inbuf.index];
    p->inbuf.ct--;
    p->inbuf.index--;
    if (p->inbuf.index < 0)
        p->inbuf.index = 0;
    
    return data;
}

void proc_buf_write(proc *p, int data)
{
    if (p->inbuf.ct == 64)
        return;
        
    p->inbuf.ct++;
    if (p->inbuf.ct == 1)
        p->inbuf.index = 0;

    p->inbuf.data[p->inbuf.index] = data;
}

void proc_brk_init(proc *p)
{
    p->brk_addr = 0x4000;
}

proc* create_proc()
{
    proc *child = (proc*)kmalloc(sizeof(proc));
    if (pq->proc_ct) // first proc use the kernel pt, no copying stack but alloc a 4K user stack space
    {
        proc_alloc_pt(child);
    } else
    {
        child->pd = kernel_pd;
        child->wdir = sys_root_dir;
    }
    proc_assign_id(child);
    proc_set_context(child);
    proc_assign_paren(child);
    proc_brk_init(child);
    proc_buf_init(child);
    link_proc(child);
    if (!pq->proc_ct)
    {
        proc_load_text(child, "/shell.bin");
    }
    pq->proc_ct++;
    return child;
}

void proc_save_context(proc *p, irq_reg_info *r)
{

    p->con.esp = r->useresp;
    p->con.ebp = r->ebp;
    p->con.eip = r->eip;
    p->con.edi = r->edi;
    p->con.esi = r->esi;
    p->con.eax = r->eax;
    p->con.ebx = r->ebx;
    p->con.ecx = r->ecx;
    p->con.edx = r->edx;
}

void proc_load_context(proc *p, irq_reg_info *r)
{
    r->useresp = p->con.esp;
    r->ebp = p->con.ebp;
    r->eip = p->con.eip;
    r->edi = p->con.edi;
    r->esi = p->con.esi;
    r->eax = p->con.eax;
    r->ebx = p->con.ebx;
    r->ecx = p->con.ecx;
    r->edx = p->con.edx;
}

void swtch(irq_reg_info *r)
{
    if (!cur_proc)
        return;
    // save current
    proc_save_context(cur_proc, r);
    // next proc
    cur_proc = cur_proc->next;
    // restore context
    proc_load_context(cur_proc, r);
    // load pdbr
    uint32_t pd_physical_addr = get_pd_physical_addr(cur_proc);
    vm_mngr_load_pd(pd_physical_addr);
}