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
            mov %0, %%edi; \
            mov %1, %%esi; \
            mov %2, %%edx; \
            mov %3, %%ecx; \
            mov %4, %%ebx; \
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


void syscall_open()
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
        : "=a"(name_addr), "=b"(attrib), "=c"(dirent_addr)
    );

    if (str_cmp("/", (char*)name_addr) == 0)
    {
        *((dirent*)dirent_addr) = sys_root_dir;
        return;
    }
    
    dirent *f = fs_find_in(&(cur_proc->wdir), (char*)name_addr, (uint8_t)attrib);
    if (f)
        *((dirent*)dirent_addr) = *f;
    else
        ((dirent*)dirent_addr)->blockno = 0;

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

void syscall_read_kbd_buf()
{
    // return to eax
    __asm__ volatile ( "mov %0, %%eax;" :: "r"(proc_buf_read(cur_proc)) );
}

void syscall_cursor_action()
{
    int option;
    __asm__ volatile ( "mov %%edi, %0;" : "=r"(option) );

    switch (option)
    {
        case 0:
            if (get_cursor_offset() != 0)
            {
                set_cursor(get_cursor_offset() - 2);
                putchar(' ');
                set_cursor(get_cursor_offset() - 2);
            }
            break;

        case 1:
            if (get_cursor_offset() > 79 * 2)
                set_cursor(get_cursor_offset() - 80 * 2);
            break;

        case 2:
            set_cursor(handle_scrolling(get_cursor_offset() + 80 * 2));
            break;
        
        case 3:
            if (get_cursor_offset() > 0)
                set_cursor(get_cursor_offset() - 2);
            break;
            
        case 4:
            set_cursor(handle_scrolling(get_cursor_offset() + 2));
            break;

        default:
            break;
    }
}

void syscall_get_cur_dir()
{
    int write_addr;
    __asm__ volatile ( "mov %%edi, %0;" : "=r"(write_addr) );
    *((dirent*)write_addr) = cur_proc->wdir;
}

void syscall_readdir()
{
    uint32_t dir_to_read;
    uint32_t index;
    uint32_t entry_to_wirte;
    __asm__ volatile 
    (
        "mov %%edi, %0;"
        "mov %%esi, %1;"
        "mov %%edx, %2;"
        : "=a"(dir_to_read), "=b"(index), "=c"(entry_to_wirte)
    );

    buf *b = bread(((dirent*)dir_to_read)->blockno);
    *((dirent*)entry_to_wirte) = ((dirent*)b->data)[index];
    brelease(b);
}

void syscall_make_dir()
{
    uint32_t name;
    __asm__ volatile ( "mov %%edi, %0;" : "=r"(name));
    if (fs_add_dir_at(&cur_proc->wdir, (char *)name))
        __asm__ volatile ("mov $1, %eax;");
    else
        __asm__ volatile ("mov $0, %eax;");
}

void syscall_change_dir()
{
    uint32_t name;
    __asm__ volatile ( "mov %%edi, %0;" : "=r"(name));

    if (str_cmp("/", (char*)name) == 0)
    {
         cur_proc->wdir = sys_root_dir;
        __asm__ volatile ("mov $1, %eax;");
        return;
    }

    dirent *e = fs_find_in(&cur_proc->wdir, (char*)name, DIRENT_ATTRIB_DIR | DIRENT_ATTRIB_USED);
    if (e)
    {
        cur_proc->wdir = *e;
        __asm__ volatile ("mov $1, %eax;");
        kfree(e);
    }
    else __asm__ volatile ("mov $0, %eax;");
}


void syscall_init()
{
    register_handler(128, syscall_handler);

    register_syscall(0, syscall_prints);
    register_syscall(1, syscall_printc);
    register_syscall(2, syscall_open);
    register_syscall(3, syscall_sbrk);
    register_syscall(4, syscall_read_kbd_buf);
    register_syscall(5, syscall_cursor_action);
    register_syscall(6, syscall_get_cur_dir);
    register_syscall(7, syscall_readdir);
    register_syscall(8, clear_screen);
    register_syscall(9, syscall_make_dir);
    register_syscall(10, syscall_change_dir);
}
