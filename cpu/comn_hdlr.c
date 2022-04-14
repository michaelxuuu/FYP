#include "comn_hdlr.h"

// static is necessary or recursive definition occurs
char* exception_msgs[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "Floating point",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

// This is a GLOBAL array, storing the addresses of intrrupt handlers defined elsewhere (in their respect .c files)
uint32_t interrupt_handlers[256];

// A common exception handler (gets called from the asm)
void _exception_handler(excp_reg_info *r) {
    printf("Interrupt: %s\n", exception_msgs[r->int_no]);
    if (r->int_no <= 31) { // exception
        // divison by 0
        if (r->int_no == 0) {
            printf("Panic!");
            __asm__ volatile("hlt");
        }
    }
}

// A common irq handler (gets called from the asm)
void _irq_handler(irq_reg_info *r) 
{
    if (r->int_no == 32)
        ;
        // printf("External Interrupt: Timer\n");
    else if (r->int_no == 33) {
        ((void(*)(irq_reg_info *))(interrupt_handlers[33]))(r);
    }
    pic_send_EOI(r->irq_no);
}

void register_handler(int i, void(*int_handler_addr)(int_reg_info *r))
{
    interrupt_handlers[i] = (uint32_t)int_handler_addr; // This type casting is unecessary but for not letting the compiler throwing a warning at us
}