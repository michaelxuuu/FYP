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
    // proc's edi, esi, edx, ecx, ebx pushed onto the stack
    // r is also passed
    ((SYSCALL_FUNC)syscalls[r->eax])(r->edi, r->esi, r->edx, r->ecx, r->ebx, (int)r);

    // obtain return val from eax and store it to the proc's eax pushed onto the kernel stack
    // so that when we have switched back to executing the interrupted proc, eax contains the return value of the syscall
    __asm__ volatile ("mov %%eax, %0":"=r"(r->eax));
}

SYSCALL1(prints, str)
{
    // use only 1 reg containing str's starting addr
    print((char*) str);
}

SYSCALL1(printc, c)
{
    // use only 1 reg containing the character
    putchar(c);
}

SYSCALL3(open, name, attrib, dirent_to_write)
{
    // use 3 regs, name str addr, attrib, and ptr to user space struct to store the file info

    if (str_cmp("/", (char*)name) == 0)
    {
        *((dirent*)dirent_to_write) = sys_root_dir;
        __asm__ volatile ("mov $1, %eax;");
        return;
    }
    
    dirent *f = fs_find_in(&(cur_proc->wdir), (char*)name, (uint8_t)attrib);

    if (f) 
    {
        *((dirent*)dirent_to_write) = *f; // file not found!
        kfree(f);
        __asm__ volatile ("mov $0, %eax;");
        return;
    }
    else 
    {
        ((dirent*)dirent_to_write)->blockno = 0;
        kfree(f);
        __asm__ volatile ("mov $1, %eax;");
        return;
    }
}

#define USER_HEAP_BASE 4096

SYSCALL1(sbrk, inc)
{
    uint32_t ubreak_addr = cur_proc->brk_addr;
    
    uint32_t prior_break = ubreak_addr;

    uint32_t block_ct = 0; // number of blocks to allocate or free

    if (inc > 0)
        block_ct = inc % 4096 > 0 ? inc / 4096 + 1 : inc / 4096; // Round up if grow
    else
        block_ct = -inc / 4096; // Round down if shrink

    for (int i = 0; i < block_ct; i++, ubreak_addr = inc > 0 ? ubreak_addr + 4096 : ubreak_addr - 4096)
        if (inc > 0)
            vm_mngr_higher_kernel_map(cur_proc->pd, ubreak_addr, pm_mngr_alloc_block(), PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER);
        else if (ubreak_addr == USER_HEAP_BASE) {__asm__ volatile ("mov $0, %eax;"); return;} // Stop freeing heap pages when hitting the heap base
        else  vm_mngr_higher_kernel_unmap(cur_proc->pd, ubreak_addr - 1);

    cur_proc->brk_addr = ubreak_addr;

    __asm__ volatile ("mov %0, %%eax;" :: "r"(prior_break));
}

SYSCALL0(read_kbd_buf)
{
    // return to eax
    __asm__ volatile ("mov %0, %%eax;" :: "r"(proc_buf_read(cur_proc)));
}

#define CURSOR_ACTION_B 0
#define CURSOR_ACTION_U 1
#define CURSOR_ACTION_D 2
#define CURSOR_ACTION_L 3
#define CURSOR_ACTION_R 4

SYSCALL1(cursor_action, opt)
{
    switch (opt)
    {
        case CURSOR_ACTION_B:
            if (get_cursor_offset() != 0)
            {
                set_cursor(get_cursor_offset() - 2);
                putchar(' ');
                set_cursor(get_cursor_offset() - 2);
            }
            break;

        case CURSOR_ACTION_U:
            if (get_cursor_offset() > 79 * 2)
                set_cursor(get_cursor_offset() - 80 * 2);
            break;

        case CURSOR_ACTION_D:
            set_cursor(handle_scrolling(get_cursor_offset() + 80 * 2));
            break;
        
        case CURSOR_ACTION_L:
            if (get_cursor_offset() > 0)
                set_cursor(get_cursor_offset() - 2);
            break;
            
        case CURSOR_ACTION_R:
            set_cursor(handle_scrolling(get_cursor_offset() + 2));
            break;

        default:
            break;
    }
}

SYSCALL1(get_cur_dir, dirent_to_write)
{
    *((dirent*)dirent_to_write) = cur_proc->wdir;
}

SYSCALL3(readdir, dir, index, dirent_to_wtite)// call open to get dirent first then call this routine to read it by index
{
    if (!index)
    {
        // report the current directory itself
        *((dirent*)dirent_to_wtite) = cur_proc->wdir;
        return;
    }
    buf *b = bread(((dirent*)dir)->blockno);
    *((dirent*)dirent_to_wtite) = ((dirent*)b->data)[--index];
    brelease(b);
}

SYSCALL1(make_dir, name)
{
    if (fs_add_dir_at(&cur_proc->wdir, (char *)name)) {__asm__ volatile ("mov $1, %eax;"); return;}
    else __asm__ volatile ("mov $0, %eax;");
}

SYSCALL1(change_dir, name)
{
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
        kfree(e);
        __asm__ volatile ("mov $1, %eax;");
        return;
    }
    else __asm__ volatile ("mov $0, %eax;");
}

SYSCALL0(fork)
{
    proc_save_context(cur_proc, (int_reg_info*)r);
    proc *p = create_proc();
    p->con.eax = 0; // child
    __asm__ volatile ("mov %0, %%eax" :: "r"(p->id)); // parent
}

SYSCALL1(exec, p)
{
    if(!proc_load_text(cur_proc, (char*)p))
        kprintf("exec failed: %s not found\n", p);
    // update parent context
    ((int_reg_info*)r)->eip = 0x1000;
    ((int_reg_info*)r)->esp = 0xbffffff4;
}

SYSCALL0(wait)
{
    cur_proc->signals[SIG_WAIT] = 1;
    swtch((int_reg_info*)r);
    __asm__ volatile ("mov %0, %%eax" :: "r"(((int_reg_info*)r)->eax)); // eax may be altered later prior to line 27
}

SYSCALL0(exit)
{
    cur_proc->parent->signals[SIG_WAIT] = 0; // ask its parent to stop waiting
    proc *temp = cur_proc;
    // schedule the next proc to run
    cur_proc = cur_proc->next;
    // restore context
    proc_load_context(cur_proc, (int_reg_info*)r);
    // load pdbr
    vm_mngr_load_pd(cur_proc->pd);
    // destory proc
    proc_destory(temp);
    __asm__ volatile ("mov %0, %%eax" :: "r"(((int_reg_info*)r)->eax)); // eax may be altered later prior to line 27
}

SYSCALL2(execv, p, argl)
{
    char *temp[10];
    for (int i = 0; i < 10; i++) temp[i] = 0;
    char **al = (char**)argl;
    for (int i = 0; al[i]; i++) 
    {
        temp[i] = kmalloc(str_len(al[i]) + 1);
        mem_copy(al[i], temp[i], str_len(al[i]) + 1);
    }
    if(!proc_load_text(cur_proc, (char*)p))
        kprintf("exec failed: %s not found\n", p);
        
    // get and args
    char **argv = (char**)0x0;
    char *argp = (char*)0xA;
    int i = 0;
    for (i = 0; temp[i]; i++, argp += (str_len(temp[i]) + 1))
    {
        mem_copy(temp[i], argp, str_len(temp[i]) + 1);
        argv[i] = argp;
    }
    
    *((uint32_t*)(0xC0000000 - 4)) = 0x0; // argv
    *((uint32_t*)(0xC0000000 - 8)) = i; // argc
    // update parent context
    ((int_reg_info*)r)->eip = 0x1000;
    ((int_reg_info*)r)->useresp = 0xbffffff4;
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
    register_syscall(11, syscall_fork);
    register_syscall(12, syscall_exec);
    register_syscall(13, syscall_wait);
    register_syscall(14, syscall_exit);
    register_syscall(15, syscall_execv);
}
