#include "comn_hdlr.h"

// A common exception handler
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

// A common irq handler
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
    interrupt_handlers[i] = (uint32_t)int_handler_addr; // This type casting is unecessary but for not letting the compiler trowing a warning at us
}