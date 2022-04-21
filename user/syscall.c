#include"syscall.h"

#define SYSCALLNUM 18

void* syscalls[SYSCALLNUM];

void register_syscall(int syscallno, void *func)
{
    syscalls[syscallno] = func;
}

void syscall_handler(int_reg_info *r)
{
    // requested syscall not exists
    if (r->eax >= SYSCALLNUM)
        return;

    // prep registers for the syscall
    // edi, esi, edx, ecx, ebx are for passing arguments
    __asm__ volatile
    (
        "\
            push %0; \
            push %1; \
            push %2; \
            push %3; \
            push %4; \
        "
        :: "r" (r->edi), "r" (r->esi), "r" (r->edx), "r" (r->ecx), "r" (r->ebx)
    );

    // do syscall
    __asm__ volatile ("call *%0" :: "r"((uint32_t)syscalls[r->eax]));
    // do not use the below way of calling in which the args will be pushed onto the stack instead of passed via registers
    // syscalls[r->eax];

    // obtain return val from eax and overwrite the user eax pushed onto the kernel stack
    // so that when we have switched back to the user space, eax contains the return value of the syscall
    __asm__ volatile ("mov %%eax, %0":"=r"(r->eax));

    // restore the stack
    for (int i = 0; i < 5; i++)
        __asm__ volatile ("pop %eax;");
}

void syscall_prints()
{
    // use only 1 reg containing str's starting addr
    char *s;
    __asm__ volatile ("mov %%edi, %0":"=r"(s));
    // call kprintf
    kprintf(s);
}

void syscall_printc()
{
    // use only 1 reg containing the character
    uint32_t c;
    __asm__ volatile ("mov %%edi, %0":"=r"(c));
    // call kprintf
    putchar(c);
}


dirent* syscall_open()
{
    // use 3 regs, name str addr, attrib, and ptr to user space struct to store the file info
    uint32_t name_addr;
    uint32_t attrib;
    uint32_t dirent_addr;
    __asm__ volatile 
    (
        "mov %%edi, %0;"
        "mov %%esi, %1;"
        "mov %%edx, %2;"
        : "=r"(name_addr), "=r"(attrib), "=r"(dirent_addr)
    );
    
    dirent *f = fs_find_in(&(cur_proc->wdir), (char*)name_addr, attrib);
    if (!f)
        return 0;
    else
        *((dirent*)dirent_addr) = *f;

    kfree(f);
}

extern uint32_t* cur_pd;

#define USER_HEAP_BASE 4096

void syscall_sbrk()
{

    cur_pd = cur_proc->pd;
    uint32_t ubreak_addr = cur_proc->brk_addr;

    int      inc;
    __asm__ volatile ( "mov %%edi, %0;" : "=r"(inc) );
    
    uint32_t prior_break = ubreak_addr;

    uint32_t block_ct = 0; // number of blocks to allocate or free

    if (inc > 0)
        block_ct = inc % 4096 > 0 ? inc / 4096 + 1 : inc / 4096; // Round up if grow
    else
        block_ct = -inc / 4096; // Round down if shrink

    for (int i = 0; i < block_ct; i++, ubreak_addr = inc > 0 ? ubreak_addr + 4096 : ubreak_addr - 4096)
        if (inc > 0)
            vm_mngr_higher_kernel_map(ubreak_addr, pm_mngr_alloc_block(), PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER);
        else if (ubreak_addr == USER_HEAP_BASE) // Stop freeing heap pages when hitting the heap base
        {
                __asm__ volatile ( "mov %0, %%eax;" :: "r"(ubreak_addr) );
                return;
        }
        else 
            vm_mngr_higher_kernel_unmap(ubreak_addr - 1);

    cur_proc->brk_addr = ubreak_addr;

    // return to eax
    __asm__ volatile ( "mov %0, %%eax;" :: "r"(prior_break) );
}

void syscall_init()
{
    register_handler(128, syscall_handler);

    register_syscall(0, syscall_prints);
    register_syscall(1, syscall_printc);
    register_syscall(2, syscall_open);
    register_syscall(3, syscall_sbrk);
}
