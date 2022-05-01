#include"tss.h"


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

task_stack_seg tss;

// System and Gate Descriptor - refer to 80386 manual Page 108 of 421 (SYSTEM SEGMENT DESCRIPTOR) AND Page 133 of 421 (TSS GATE DESCRIPTOR)

typedef struct gdt_ent {
	unsigned int limit_low              : 16;
	unsigned int base_low               : 24;
	unsigned int accessed               :  1;
	unsigned int read_write             :  1; // readable for code, writable for data
	unsigned int conforming_expand_down :  1; // conforming for code, expand down for data
	unsigned int code                   :  1; // 1 for code, 0 for data
	unsigned int code_data_segment      :  1; // should be 1 for everything but TSS and LDT
	unsigned int DPL                    :  2; // privilege level
	unsigned int present                :  1;
	unsigned int limit_high             :  4;
	unsigned int available              :  1; // only used in software; has no effect on hardware
	unsigned int long_mode              :  1;
	unsigned int big                    :  1; // 32-bit opcodes for code, uint32_t stack for data
	unsigned int gran                   :  1; // 1 to use 4k page addressing, 0 for byte addressing
	unsigned int base_high              :  8;
} __attribute__((packed)) gdt_ent;

void flush_tss()
{
	__asm__ volatile 
	( 
		"mov $tss_selector, %ax;"
		"or $3, %ax;"
		" ltr %ax;" 
	);
}

void write_tss(gdt_ent *g) {
	// Compute the base and limit of the TSS for use in the GDT entry.
	uint32_t base = (uint32_t) &tss;
	uint32_t limit = sizeof(tss);
 
	// Add a TSS descriptor to the GDT.
	g->limit_low = limit;
	g->base_low = base;
	g->accessed = 1; // With a system entry (`code_data_segment` = 0), 1 indicates TSS and 0 indicates LDT
	g->read_write = 0; // For a TSS, indicates busy (1) or not busy (0).
	g->conforming_expand_down = 0; // always 0 for TSS
	g->code = 1; // For a TSS, 1 indicates 32-bit (1) or 16-bit (0).
	g->code_data_segment=0; // indicates TSS/LDT (see also `accessed`)
	g->DPL = 0; // ring 0, see the comments below
	g->present = 1;
	g->limit_high = (limit & (0xf << 16)) >> 16; // isolate top nibble
	g->available = 0; // 0 for a TSS
	g->long_mode = 0;
	g->big = 0; // should leave zero according to manuals.
	g->gran = 0; // limit is in bytes, not pages
	g->base_high = (base & (0xff << 24)) >> 24; //isolate top byte
 
	// Ensure the TSS is initially zero'd.
	mem_set(&tss, 0, sizeof(tss));
 
	tss.ss0  = (uint32_t)&kdata_selector;  // Set the kernel stack segment.
	tss.esp0 = 0; // Set the kernel stack pointer.
	//note that CS is loaded from the IDT entry and should be the regular kernel code segment

	tss.cs = (uint32_t)(&kcode_selector) | 3; // doing an or with 0b11 setting the last two bit of a selector (RPL requested privilege level) - refer to refer to 80386 manual Page 96 of 421
	tss.ss = tss.ds = tss.es = tss.fs = tss.gs = (uint32_t)(&kdata_selector) | 3;

	flush_tss();
}

// Call this to set kernel esp so that the esp is at a known address the next time the kernel is executed
// eg. the arguments can be pushed to the correct position on the kernel stack the next time when a process invokes a system call 
void tss_set_esp()
{
	__asm__ volatile ("mov %%esp, %0":"=r"(tss.esp0));
}

// i386 does not support swicthing to the segment (as user code segemnet) of a numerically greater privilege level
// Therefore, to swicth to user mode, we have to get around it using the techique of returning to user mode using 'iret' as if we just got to the kernel mode because of an user level trap/ interrupt/ syscall
// 'iret' causes the processor to jump to cs:eip, which we have to have prepared in the stack. It also sets the EFLAGS register with the value above from the stack. 
// ss:esp will be set to point to the SS and ESP values that was obtained from the stack.

void goto_user()
{
	write_tss((gdt_ent*)(&tss_descriptor));
	tss_set_esp();
	__asm__ volatile
	(
		"cli;"
		// as if we are "restoring" the user/ kernel
		"mov $udata_selector, %ax;"
		"or $3, %ax;"
		"mov %ax, %dx;"
		"mov %ax, %es;"
		"mov %ax, %fs;"
		"mov %ax, %gs;"
		// pretend to be the cpu and push the default registers onto the stack for iret to pop

		// push user ss (with RDP set to 3, simulating that we were in the user mode and got into the kernel mode by interrupt)
		"mov $udata_selector, %eax;"
		"or $3, %eax;"
		"pushl %eax;"
		// push esp
		"pushl $0xbffffff4;"
		// push flags
		"pushf;"
		"pop %eax;"
		"or $0x200, %eax;"
		"push %eax;"
		// push user cs (with RDP set to 3, simulating that we were in the user mode and got into the kernel mode by interrupt)
		"mov $ucode_selector, %eax;"
		"or $3, %eax;"
		"pushl %eax;"
		// push eip which is the eip we had before the pretend interrupt in the user mode (we set it to a known place - the entry point to the first proc)
		"pushl $0x0;"
		"iret;"
	);
}

