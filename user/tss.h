#ifndef TSS_H
#define TSS_H

#include"../include/type.h"

#include"../Lib/util.h"

typedef struct task_stack_seg {
	uint32_t prevTss;
	uint32_t esp0;              // The stack pointer to load when we change to kernel mode.
	uint32_t ss0;               // The stack segment to load when we change to kernel mode.
	uint32_t esp1;
	uint32_t ss1;
	uint32_t esp2;
	uint32_t ss2;
	uint32_t cr3;
	uint32_t eip;
	uint32_t eflags;
	uint32_t eax;
	uint32_t ecx;
	uint32_t edx;
	uint32_t ebx;
	uint32_t esp;
	uint32_t ebp;
	uint32_t esi;
	uint32_t edi;
	uint32_t es;				// The value to load into ES when we change to kernel mode.
	uint32_t cs;				// The value to load into CS when we change to kernel mode.
	uint32_t ss;				// The value to load into SS when we change to kernel mode.
	uint32_t ds;				// The value to load into DS when we change to kernel mode.
	uint32_t fs;				// The value to load into FS when we change to kernel mode.
	uint32_t gs;				// The value to load into GS when we change to kernel mode.
	uint32_t ldt;
	uint16_t trap;
	uint16_t iomap;
}__attribute__((packed)) task_stack_seg;

void install_tss();

// Call this to set kernel esp so that the esp is at a known address the next time the kernel is executed
// eg. the arguments can be pushed to the correct position on the kernel stack the next time when a process invokes a system call 
void tss_set_kernel_esp(uint32_t esp);

#endif