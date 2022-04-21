#include "idt.h"

idt_gate idt[256];

idt_descriptor idt_descriptor_sturct;

void load_idtr() {
    idt_descriptor_sturct.base = ((uint32_t)&(idt));
    idt_descriptor_sturct.limit = sizeof(idt_gate) * 256 - 1;
    /**
     * With paging enabled and the GDT base address field being 0, the logical address of the idt 
     * equals its linear address. (Refer to page 91 of 421 in the i386 manual)
     * Thus, given the I386 manual states that IDTR stores the linear address of the idt base,
     * and that the idt descriptor base address field's been installed with the virtual (logical) address,
     * we should directly load the entire idt descriptor base address of 32 bits into IDTR.
     * 
     * This is done easily using lidtl, where the suffix - l - tells indicates a oprand size of 32 bits 
     * and to load the entire 24 bits of the diescriptor base field to IDTR.
     */
    __asm__ volatile("lidtl (%0)" : : "r"(&idt_descriptor_sturct));
}

extern uint32_t kcode_selector;

void register_idt_gate(int gate_num, uint32_t handler_addr, int syscall) {
    idt[gate_num].handler_addr_low = low_16(handler_addr);
    idt[gate_num].handler_seg_selector = (uint8_t)((uint32_t)(&kcode_selector)); // Marked as entern in .h file, defined in kernel_entry.s
    idt[gate_num].always0 = 0;
    idt[gate_num].flags = syscall ? (0x8E | 0x60) : 0x8E;
    idt[gate_num].handler_addr_high = high_16(handler_addr);
}

// A function installing the idt with all gates needed
void install_idt() {

    register_idt_gate(0, (uint32_t) int_0, 0);
    register_idt_gate(1, (uint32_t) int_1, 0);
    register_idt_gate(2, (uint32_t) int_2, 0);
    register_idt_gate(3, (uint32_t) int_3, 0);
    register_idt_gate(4, (uint32_t) int_4, 0);
    register_idt_gate(5, (uint32_t) int_5, 0);
    register_idt_gate(6, (uint32_t) int_6, 0);
    register_idt_gate(7, (uint32_t) int_7, 0);
    register_idt_gate(8, (uint32_t) int_8, 0);
    register_idt_gate(9, (uint32_t) int_9, 0);
    register_idt_gate(10, (uint32_t) int_10, 0);
    register_idt_gate(11, (uint32_t) int_11, 0);
    register_idt_gate(12, (uint32_t) int_12, 0);
    register_idt_gate(13, (uint32_t) int_13, 0);
    register_idt_gate(14, (uint32_t) int_14, 0);
    register_idt_gate(15, (uint32_t) int_15, 0);
    register_idt_gate(16, (uint32_t) int_16, 0);
    register_idt_gate(17, (uint32_t) int_17, 0);
    register_idt_gate(18, (uint32_t) int_18, 0);
    register_idt_gate(19, (uint32_t) int_19, 0);
    register_idt_gate(20, (uint32_t) int_20, 0);
    register_idt_gate(21, (uint32_t) int_21, 0);
    register_idt_gate(22, (uint32_t) int_22, 0);
    register_idt_gate(23, (uint32_t) int_23, 0);
    register_idt_gate(24, (uint32_t) int_24, 0);
    register_idt_gate(25, (uint32_t) int_25, 0);
    register_idt_gate(26, (uint32_t) int_26, 0);
    register_idt_gate(27, (uint32_t) int_27, 0);
    register_idt_gate(28, (uint32_t) int_28, 0);
    register_idt_gate(29, (uint32_t) int_29, 0);
    register_idt_gate(30, (uint32_t) int_30, 0);
    register_idt_gate(31, (uint32_t) int_31, 0);
    register_idt_gate(32, (uint32_t) int_32, 0);
    register_idt_gate(33, (uint32_t) int_33, 0);
    register_idt_gate(34, (uint32_t) int_34, 0);
    register_idt_gate(35, (uint32_t) int_35, 0);
    register_idt_gate(36, (uint32_t) int_36, 0);
    register_idt_gate(37, (uint32_t) int_37, 0);
    register_idt_gate(38, (uint32_t) int_38, 0);
    register_idt_gate(39, (uint32_t) int_39, 0);
    register_idt_gate(40, (uint32_t) int_40, 0);
    register_idt_gate(41, (uint32_t) int_41, 0);
    register_idt_gate(42, (uint32_t) int_42, 0);
    register_idt_gate(43, (uint32_t) int_43, 0);
    register_idt_gate(44, (uint32_t) int_44, 0);
    register_idt_gate(45, (uint32_t) int_45, 0);
    register_idt_gate(46, (uint32_t) int_46, 0);
    register_idt_gate(47, (uint32_t) int_47, 0);
    register_idt_gate(0x80, (uint32_t) int_128, 1);
}

void interrupt_init() {

    // initialize 8259A PIC
    pic_init();

    install_idt();

    // put idt descriptor into lidt register
    load_idtr();

    // Enable interrupt
    __asm__ volatile ("sti;");
}