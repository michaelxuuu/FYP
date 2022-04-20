// Interrupt -> exception (internal/ software interrupt) + irq (external/ hardware interrupt)

.extern _exception_handler  // C level exception handler defined in another translation unit
.extern _irq_handler        // C level irq handler defined in another translation unit

// exception without cpu automatically pushing the error code onto the kernel stack
.macro EXCEPTION_WITHOUT_ERRCODE, int_n
    .global int_\int_n
    int_\int_n:
        pushl $0
        pushl $\int_n
        jmp handle_exception
.endm

// exception with cpu automatically pushing the error code onto the kernel stack
.macro EXCEPTION_WITH_ERRCODE, int_n
    .global int_\int_n
    int_\int_n:
        pushl $\int_n
        jmp handle_exception
.endm

// interrupt reequests from the PIC
.macro IRQ, irq_n, int_n
    .global int_\int_n
    int_\int_n:
        pushl $\irq_n // IRQ number from 0 - 15
        pushl $\int_n // interrupt number starting from 32
        jmp handle_irq
.endm

/* Here interrupt handlers are defined in assambly
assambly is necessary here cuz C calling convention changes some registers
so if we write the exception handler in C, we would fail to get the desired registers values */

// IVT 0 - IVT 31 : Protected Mode Processor-generated Exceptions (Reserved by Intel)
EXCEPTION_WITHOUT_ERRCODE 0   // division by 0
EXCEPTION_WITHOUT_ERRCODE 1   // debug exception (Single-step interrupt)
EXCEPTION_WITHOUT_ERRCODE 2   // non-maskable exception (NMI)
EXCEPTION_WITHOUT_ERRCODE 3   // Breakpoint
EXCEPTION_WITHOUT_ERRCODE 4   // Overflow
EXCEPTION_WITHOUT_ERRCODE 5   // Bound Range Exceeded
EXCEPTION_WITHOUT_ERRCODE 6   // Invalid Opcode
EXCEPTION_WITHOUT_ERRCODE 7   // Coprocessor not available
EXCEPTION_WITH_ERRCODE    8   // Double fault
EXCEPTION_WITHOUT_ERRCODE 9   // Coprocessor Segment Overrun
EXCEPTION_WITH_ERRCODE    10  // 10: Bad TSS Exception
EXCEPTION_WITH_ERRCODE    11  // 11: Segment Not Present 
EXCEPTION_WITH_ERRCODE    12  // 12: Stack Fault
EXCEPTION_WITH_ERRCODE    13  // 13: General Protection Fault
EXCEPTION_WITH_ERRCODE    14  // Page fault
EXCEPTION_WITHOUT_ERRCODE 15  // Reserved
EXCEPTION_WITHOUT_ERRCODE 16  // Floating point exception
EXCEPTION_WITHOUT_ERRCODE 17  // Alignment check exception
EXCEPTION_WITHOUT_ERRCODE 18  // Machine check exception
EXCEPTION_WITHOUT_ERRCODE 19  // Reserved
EXCEPTION_WITHOUT_ERRCODE 20  // Reserved
EXCEPTION_WITHOUT_ERRCODE 21  // Reserved
EXCEPTION_WITHOUT_ERRCODE 22  // Reserved
EXCEPTION_WITHOUT_ERRCODE 23  // Reserved
EXCEPTION_WITHOUT_ERRCODE 24  // Reserved
EXCEPTION_WITHOUT_ERRCODE 25  // Reserved
EXCEPTION_WITHOUT_ERRCODE 26  // Reserved
EXCEPTION_WITHOUT_ERRCODE 27  // Reserved
EXCEPTION_WITHOUT_ERRCODE 28  // Reserved
EXCEPTION_WITHOUT_ERRCODE 29  // Reserved
EXCEPTION_WITHOUT_ERRCODE 30  // Reserved
EXCEPTION_WITHOUT_ERRCODE 31  // Reserved

EXCEPTION_WITHOUT_ERRCODE 128  // SYSCALL

// IRQ's
// Each interrupt line of IRQ corresponds to one signle IRQ
// At total: 15 types of interrupt (error code 0 to 15)

IRQ 0,  32  // IRQ 0 - sys timer
IRQ 1,  33  // IRQ 1 - keyboard on PS/2 port
IRQ 2,  34  // IRQ 2 - cascaded signals from IRQs 8–15
IRQ 3,  35  // IRQ 3
IRQ 4,  36  // IRQ 4
IRQ 5,  37  // IRQ 5
IRQ 6,  38  // IRQ 6
IRQ 7,  39  // IRQ 7
IRQ 8,  40  // IRQ 8
IRQ 9,  41  // IRQ 9
IRQ 10, 42  // IRQ 10
IRQ 11, 43  // IRQ 11
IRQ 12, 44  // IRQ 12
IRQ 13, 45  // IRQ 13
IRQ 14, 46  // IRQ 14
IRQ 15, 47  // IRQ 15


handle_exception:
    cli
    // save cpu state
    pusha
    // save user/kernel data segement
    // kenel code segment is automatically loaded into cs register by the hardware when interrupts occur
    mov %ds, %ax
    pushl %eax
    // load kernel ds
    mov $kdata_selector, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    // call C-level handler
    pushl %esp // pass parameter
    call _exception_handler
    pop %eax

    // restore user/kernel ds
    pop %eax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    // retore cpu state
    popa
    // cleans up the pushed error code as well as the interrupt number
    add $8, %esp
    // use iret (return from interrupt)
    // pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP
    // where CS, EIP, EFLAGS, SS, and ESP are automatically pushed by the cpu
    sti
    iret

handle_irq:
    cli
    // save cpu state
    pusha
    // save user/kernel data segement
    // kenel code segment is automatically loaded into cs register by the hardware when interrupts occur
    mov %ds, %ax
    pushl %eax 
    // load kernel ds
    mov $kdata_selector, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    // call C-level handler
    pushl %esp // pass parameter
    call _irq_handler
irq_back:
    pop %eax

    // restore user/kernel ds
    pop %eax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    // retore cpu state
    popa
    // cleans up the pushed error code as well as the interrupt number
    add $8, %esp
    // use iret (return from interrupt)
    // pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP
    // where CS, EIP, EFLAGS, SS, and ESP are automatically pushed by the cpu
    sti
    iret