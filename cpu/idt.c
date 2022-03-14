#include "idt.h"

void load_idt() {
    idt_descriptor_sturct.base = (uint32_t) &(idt);
    idt_descriptor_sturct.limit = sizeof(idt_gate) * 256 - 1;
    __asm__ volatile("lidt (%0)" : : "r"(&idt_descriptor_sturct));
}

void register_idt_gate(int gate_num, uint32_t handler_addr) {
    idt[gate_num].handler_addr_low = low_16(handler_addr);
    idt[gate_num].handler_seg_selector = 0x08; // kernel code segment offset
    idt[gate_num].always0 = 0;
    idt[gate_num].flags = 0x8E;
    idt[gate_num].handler_addr_high = high_16(handler_addr);
}

// A function installing the idt with all gates needed
void install_idt() {

    // initialize 8259A PIC
    pic_init();

    register_idt_gate(0, (uint32_t) int_0);
    register_idt_gate(1, (uint32_t) int_1);
    register_idt_gate(2, (uint32_t) int_2);
    register_idt_gate(3, (uint32_t) int_3);
    register_idt_gate(4, (uint32_t) int_4);
    register_idt_gate(5, (uint32_t) int_5);
    register_idt_gate(6, (uint32_t) int_6);
    register_idt_gate(7, (uint32_t) int_7);
    register_idt_gate(8, (uint32_t) int_8);
    register_idt_gate(9, (uint32_t) int_9);
    register_idt_gate(10, (uint32_t) int_10);
    register_idt_gate(11, (uint32_t) int_11);
    register_idt_gate(12, (uint32_t) int_12);
    register_idt_gate(13, (uint32_t) int_13);
    register_idt_gate(14, (uint32_t) int_14);
    register_idt_gate(15, (uint32_t) int_15);
    register_idt_gate(16, (uint32_t) int_16);
    register_idt_gate(17, (uint32_t) int_17);
    register_idt_gate(18, (uint32_t) int_18);
    register_idt_gate(19, (uint32_t) int_19);
    register_idt_gate(20, (uint32_t) int_20);
    register_idt_gate(21, (uint32_t) int_21);
    register_idt_gate(22, (uint32_t) int_22);
    register_idt_gate(23, (uint32_t) int_23);
    register_idt_gate(24, (uint32_t) int_24);
    register_idt_gate(25, (uint32_t) int_25);
    register_idt_gate(26, (uint32_t) int_26);
    register_idt_gate(27, (uint32_t) int_27);
    register_idt_gate(28, (uint32_t) int_28);
    register_idt_gate(29, (uint32_t) int_29);
    register_idt_gate(30, (uint32_t) int_30);
    register_idt_gate(31, (uint32_t) int_31);
    register_idt_gate(32, (uint32_t) int_32);
    register_idt_gate(33, (uint32_t) int_33);
    register_idt_gate(34, (uint32_t) int_34);
    register_idt_gate(35, (uint32_t) int_35);
    register_idt_gate(36, (uint32_t) int_36);
    register_idt_gate(37, (uint32_t) int_37);
    register_idt_gate(38, (uint32_t) int_38);
    register_idt_gate(39, (uint32_t) int_39);
    register_idt_gate(40, (uint32_t) int_40);
    register_idt_gate(41, (uint32_t) int_41);
    register_idt_gate(42, (uint32_t) int_42);
    register_idt_gate(43, (uint32_t) int_43);
    register_idt_gate(44, (uint32_t) int_44);
    register_idt_gate(45, (uint32_t) int_45);
    register_idt_gate(46, (uint32_t) int_46);
    register_idt_gate(47, (uint32_t) int_47);

    // put idt descriptor into lidt register
    load_idt();
}
