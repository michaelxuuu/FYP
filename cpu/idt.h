#ifndef IDT_H
#define IDT_H

#include "stdint.h"
#include "pic.h"

// Interrupt Descriptor Table -> 256 vectors
// INT 0 - INT 255:
//
// INT 0 - INT 31 -> Protected mode exceptions reserved by Intel:
// INT 0 => Division by zero
// ...
//
// INT 32 - INT 47 => IRQ 0 - IRQ 15:
// INT 32 => IRQ 0 – system timer (cannot be changed)
// INT 33 => IRQ 1 – keyboard on PS/2 port (cannot be changed)
// INT 34 => IRQ 2 – cascaded signals from IRQs 8–15 (any devices configured to use IRQ 2 will actually be using IRQ 9)
// ...

// Handlers defined externally in assambly - see 'int.s'
extern void int_0();
extern void int_1();
extern void int_2();
extern void int_3();
extern void int_4();
extern void int_5();
extern void int_6();
extern void int_7();
extern void int_8();
extern void int_9();
extern void int_10();
extern void int_11();
extern void int_12();
extern void int_13();
extern void int_14();
extern void int_15();
extern void int_16();
extern void int_17();
extern void int_18();
extern void int_19();
extern void int_20();
extern void int_21();
extern void int_22();
extern void int_23();
extern void int_24();
extern void int_25();
extern void int_26();
extern void int_27();
extern void int_28();
extern void int_29();
extern void int_30();
extern void int_31();
extern void int_32();
extern void int_33();
extern void int_34();
extern void int_35();
extern void int_36();
extern void int_37();
extern void int_38();
extern void int_39();
extern void int_40();
extern void int_41();
extern void int_42();
extern void int_43();
extern void int_44();
extern void int_45();
extern void int_46();
extern void int_47();

// Define idt gate
typedef struct idt_gate
{
    uint16_t handler_addr_low;
    uint16_t handler_seg_selector; // kernel code segment where the handler code resides
    uint8_t always0;
    uint8_t flags;
    // High to low
    // P: gate is active ? -> Yes -> set to 1
    // DPL: what privilege is required to invoke the handler -> 00
    // 0: just 0 for no reason
    // D: the code segment to jump to is 32-bit ? -> Yes -> 1
    // TYPE : gate type -> 110 indicating interrupt gate
    uint16_t handler_addr_high;
} __attribute__((packed)) idt_gate;

// Define idt that contains 256 idt gates
extern idt_gate idt[256];

// A IDT descriptor that "lidt" will read
typedef struct idt_descriptor {
    uint16_t limit;
    uint32_t base; 
} __attribute__((packed)) idt_descriptor;

extern idt_descriptor idt_descriptor_sturct;

// Function that loads the idt
void load_idtr();

// Function that registers an IDT gate with a handler in IDT
void register_idt_gate(int gate_num, uint32_t handler_addr);

// Function that registers the idt gates and loads idt
void install_idt();

#endif