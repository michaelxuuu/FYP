#ifndef PROC_H
#define PROC_H

#include"../include/type.h"

#include"../kernel/kmalloc.h"

#include"../mem/vm_mngr.h"

#include"../mem/pm_mngr.h"

#include"../cpu/comn_hdlr.h"

#include"../cpu/idt.h"

#include"tss.h"


typedef struct context {
    uint32_t esp;
    uint32_t ebp;
    uint32_t eip;
    uint32_t edi;
    uint32_t esi;
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
} context;

// This proc struct is managed by the kernel
typedef struct proc proc;
typedef struct proc
{
    uint32_t id;    // unique id
    uint32_t *pd;   // page directory (table)
    context con;    // context (register state)
    proc *parent;   // parent proc

    proc *next;
    proc *prev;
}proc;

extern uint32_t *cur_pd;        // defined in vm_mngr.c

extern uint32_t *kernel_pd;     // defined in vm_mngr.c

typedef struct procqueue
{
    proc *cur;
    proc *head;
    uint32_t proc_ct;
}procqueue;

extern procqueue *pq;

// Link proc to the end of queue of proc's (fifo)
void link_proc(proc *p);

extern uint32_t nextid;

void proc_zero_context(proc *p);

void proc_init();

// pt is inherited from the parent proc... which is the currently running proc
void proc_alloc_pt(proc *child);

void proc_destory_pt(proc *p);

void proc_assign_id(proc *p);

void proc_assign_paren(proc *p);

extern proc *cur_proc; // pointer to the currently running process

proc* create_proc();

void swtch(irq_reg_info *r);

#endif