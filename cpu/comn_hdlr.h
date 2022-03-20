#ifndef COMN_HDLR_H
#define COMN_HDLR_H

#include <stdint.h>
#include "idt.h"
#include "./../drivers/screen.h"

// A struct saving the register status when an interrupt occurs
// When an interrupt occurs, some registers will be pushed onto the stack
// 1. eip, cs, eflags, useresp, ss are pushed automatically by the cpu
// 2. int_no, err_code, where the int_no is user-defined and the err_code is automatically pushed by the CPU for some particlar exceptions
// (Here the handler will get called where the kernel mode starts)
// 3. edi, esi, ebp, esp, ebx, edx, ecx, eax are pushed by pusha used by c function calling mechanism
// 4. ds is pushed by the os for later restoring of the user data seg from the kernel data seg
typedef struct {
    uint32_t ds; /* Data segment selector */
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; /* Pushed by pusha. */
    uint32_t int_no, err_code; /* Interrupt number and error code (if applicable) */
    uint32_t eip, cs, eflags, useresp, ss; /* Pushed by the processor automatically */
}  excp_reg_info;

typedef struct {
    uint32_t ds; /* Data segment selector */
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; /* Pushed by pusha. */
    uint32_t int_no, irq_no; /* Interrupt number and error code (if applicable) */
    uint32_t eip, cs, eflags, useresp, ss; /* Pushed by the processor automatically */
}  irq_reg_info;

typedef irq_reg_info int_reg_info;

// static is necessary or recursive definition occurs
extern char* exception_msgs[];

// This is a GLOBAL array, storing the addresses of intrrupt handlers defined elsewhere (in their respect .c files)
extern uint32_t interrupt_handlers[256];

// A common exception handler
void _exception_handler(excp_reg_info *r);

// A common irq handler
void _irq_handler(irq_reg_info *r);

void register_handler(int, void(*int_handler_addr)(int_reg_info *));

#endif