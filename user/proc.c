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

void proc_copy_context(proc *p)
{
    p->con = cur_proc->con;
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
        vm_mngr_higher_kernel_map(KERNEL_PD_BASE, i * 4096, pm_mngr_alloc_block(), PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER);
    // proc heap
    vm_mngr_higher_kernel_map(KERNEL_PD_BASE, 0x4000, pm_mngr_alloc_block(), PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER);
    // proc stack
    vm_mngr_higher_kernel_map(KERNEL_PD_BASE, 0xC0000000 - 0x1000, pm_mngr_alloc_block(), PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER);


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

void proc_alloc_stack(proc *child)
{
    uint32_t *cpd = vm_mngr_map_frame(child->pd);
    uint32_t stack_va = 0xC0000000 - 0x1000;
    uint32_t stack_dir_index = va_get_dir_index(stack_va);
    uint32_t stack_page_index = va_get_page_index(stack_va);

    // get parent proc's stack
    uint32_t *pt = vm_mngr_map_frame(page_get_frame_addr(cpd[stack_dir_index])); // now cpd (child page dir) is the same to ppd (paren page dir)
    uint32_t *paren_stack = vm_mngr_map_frame(pt[stack_page_index]);

    // unlink the parent proc's stack physical frame from the child page directory
    cpd[stack_dir_index] = 0;

    // allocate a new page for the child stack
    uint32_t child_stack_pa = pm_mngr_alloc_block();
    vm_mngr_higher_kernel_map(child->pd, stack_va, child_stack_pa, PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER);
    uint32_t *child_stack = vm_mngr_map_frame(child_stack_pa);

    // copy the parent proc's stack to the child
    mem_copy((char*)paren_stack, (char*)child_stack, 4096);
    
    vm_mngr_unmap_frame(cpd);
    vm_mngr_unmap_frame(pt);
    vm_mngr_unmap_frame(child_stack);
    vm_mngr_unmap_frame(paren_stack);
}

void proc_alloc_text(proc *child)
{
    uint32_t *pd = vm_mngr_map_frame(child->pd);

    // unlink the parent proc's text physical frame from the child page directory
    pd[0] = 0;

    // allocate a new page for the child text
    for (int i = 0; i < 4; i++) {
        invalidate_tlb_ent(i * 4096);
        vm_mngr_higher_kernel_map(child->pd, i * 4096, pm_mngr_alloc_block(), PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER);
    }

    vm_mngr_unmap_frame(pd);
}

// pt is inherited from the parent proc... which is the currently running proc
void proc_alloc_pt(proc *child)
{

    // Create a page diretcory for the child proc and copy the parent proc's pd to it

    // child
    child->pd = pm_mngr_alloc_block();
    uint32_t *cpd = vm_mngr_map_frame(child->pd);
    // parent
    uint32_t *ppd = vm_mngr_map_frame(child->parent->pd);
    // copy!
    mem_copy((char*)ppd, (char*)cpd, 4096);
    vm_mngr_unmap_frame(cpd);
    vm_mngr_unmap_frame(ppd);
}

void proc_destory_pt(proc *p)
{
    // free text and heap
    for (int i = 0; i < 4; i++)
        vm_mngr_higher_kernel_unmap(p->pd, i * 4096);
    // free stack
    vm_mngr_higher_kernel_unmap(p->pd, 0xC0000000 - 0x1000);
    // free pd
    pm_mngr_free_block(p->pd);
}

void proc_unlink(proc *p)
{
    p->prev->next = p->next;
    p->next->prev = p->prev;
    kfree(p);
}

void proc_destory(proc *p)
{
    proc_destory_pt(p);
    proc_unlink(p);
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

int proc_load_text(proc *p, char* path)
{
    dirent *e = fs_find(path, DIRENT_ATTRIB_USED);
    if (!e)
        return 0; // exec failed
    
    // clear old text
    proc_alloc_text(cur_proc);
    for (uint32_t blockno = e->blockno, add_to_wirte = 0x0; blockno != 0; blockno = fat_get_next(blockno), add_to_wirte += 4096)
    {
        buf *b = bread(blockno);
        mem_copy(b->data, (char*)add_to_wirte, 4096);
        brelease(b);
    }
    kfree(e);
    return 1;
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

void proc_sig_init(proc *p)
{
    for (int i = 0; i < SIG_NUM; i++)
        p->signals[i] = 0;
}

proc* create_proc()
{
    proc *child = (proc*)kmalloc(sizeof(proc));
    proc_assign_paren(child);
    if (pq->proc_ct)
    {
        proc_alloc_pt(child);
        proc_alloc_stack(child);
        child->wdir = child->parent->wdir;
    }
    else // first proc use the kernel pt, no copying stack but alloc a 4K user stack space
    {
        child->pd = KERNEL_PD_BASE;
        child->wdir = sys_root_dir;
    }
    proc_assign_id(child);
    if (pq->proc_ct) proc_copy_context(child);
    else proc_set_context(child);
    proc_brk_init(child);
    proc_buf_init(child);
    proc_sig_init(child);
    link_proc(child);
    if (!pq->proc_ct)
        proc_load_text(child, "/shell.bin");
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
    if (cur_proc->next->signals[SIG_WAIT]) // if the next proc is waiting on its child proc
        return;
    // save current
    proc_save_context(cur_proc, r);
    // next proc
    cur_proc = cur_proc->next;
    // restore context
    proc_load_context(cur_proc, r);
    // load pdbr
    vm_mngr_load_pd(cur_proc->pd);
}