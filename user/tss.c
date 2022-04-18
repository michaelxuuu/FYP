#include"tss.h"

task_stack_seg tss;

// System and Gate Descriptor - refer to 80386 manual Page 108 of 421 (SYSTEM SEGMENT DESCRIPTOR) AND Page 133 of 421 (TSS GATE DESCRIPTOR)

// kcode_descriptor:
//     .word 0xffff        // seglim_0_15
//     .word 0x0           // base_0_15
//     .byte 0x0           // base_16_23
//     .byte 0b10011010    // low -> high: type (Accessed - 0, Code - 1, Conforming - 0, Readable - 1), descriptor_type - 1, Privilige - 00, Present - 1 
//     .byte 0b11001111    // low -> high: seg_lim_16_19 - 0b1111, AVL - 0, always_0 - 0, always_1 - 1, Granularity - 0 (byte offset!!!)
//     .byte 0x0           // base_24_31

typedef struct gdt_ent
{
   uint16_t limit_low;           // The lower 16 bits of the limit.
   uint16_t base_low;            // The lower 16 bits of the base.
   uint8_t  base_middle;         // The next 8 bits of the base.
   uint8_t  access;              // Access flags, determine what ring this segment can be used in.
   uint8_t  granularity;
   uint8_t  base_high;           // The last 8 bits of the base.
} __attribute__((packed)) gdt_ent;

void gdt_set_gate(gdt_ent *e, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
	e->base_low    = (base & 0xFFFF);
	e->base_middle = (base >> 16) & 0xFF;
	e->base_high   = (base >> 24) & 0xFF;
	e->limit_low   = (limit & 0xFFFF);
	e->granularity = (limit >> 16) & 0x0F;
	e->granularity |= gran & 0xF0;
	e->access      = access;
}

/* External veriables defined in kernel_entry.s */
extern uint32_t kcode_selector;
extern uint32_t kdata_selector;
extern uint32_t tss_descriptor;
extern uint32_t tss_selector;

void install_tss()
{
	// clear tss
	mem_set(&tss, 0, sizeof(tss));

	// Install the tss descriptor in gdt
	gdt_ent *e = (gdt_ent*)(&tss_descriptor);
	gdt_set_gate(e, (uint32_t)&tss, base + sizeof(tss), 0xE9, 0xCF);

	tss.ss0 = (uint32_t)&kdata_selector;
	tss.esp0 = 0; // Initialized to 0, will be assigned with a valid value when existing from kernel
	tss.cs = (uint32_t)(&kcode_selector) | 0b11; // doing an or with 0b11 setting the last two bit of a selector (RPL requested privilege level) - refer to refer to 80386 manual Page 96 of 421
	tss.ss = tss.ds = tss.es = tss.fs = tss.gs = (uint32_t)(&kdata_selector) | 0b11;

	__asm__ volatile ( "mov $tss_selector, %ax; ltr %ax" );
}

// Call this to set kernel esp so that the esp is at a known address the next time the kernel is executed
// eg. the arguments can be pushed to the correct position on the kernel stack the next time when a process invokes a system call 
void tss_set_kernel_esp(uint32_t esp)
{
	tss.esp0 = esp;
}