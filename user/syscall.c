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

    // restore the stack
    for (int i = 0; i < 5; i++)
        __asm__ volatile ("pop %eax;");

    // obtain return val from eax and overwrite the user eax pushed onto the kernel stack
    // so that when we have switched back to the user space, eax contains the return value of the syscall
    __asm__ volatile ("mov %%eax, %0":"=r"(r->eax));
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

void syscall_init()
{
    register_handler(128, syscall_handler);

    register_syscall(0, syscall_prints);
    register_syscall(1, syscall_printc);
    register_syscall(2, syscall_open);
}
