/*

    Here, we have 3 objectives:

        1. Set up the GDT
        2. Enable A20 address line
        3. Switch to 32bit Protected-Mode!

    GDT: global descriptor table

    One gdt entry describes one memory segment and its attributes such as write or read or execute 
    permissions, used to serve as the memory protection mechanism before the paging scheme was intro-
    -duced. Now, we adopt flat memory model, which says that the entire 4G addressable memory is flat,
    and its size of 4G is just the size of one segment.

    Here we define two segmnets: code and data segment. Why two? Well, recall, in 16bit read mode, the
    CPU fetches instructions at CS:IP, and it loads data from DS:offset where the offset could be in a 
    register or an immediate. Now, in the 32bit mode we are about to swicth to, this instruction fetching
    and segment register-offset data accessing scheme will be the same EXCEPT the value of the CS register
    is now changed from the data sgement base address to a code segment selector pointing to a GDT entry 
    that represents the code segment, in which the sgement base address, limit and other attributes such as 
    read permissions are specified, and that the value of the DS register is now changed from the data
    sgement base address to a data segment selector pointing to another GDT entry that represents the data 
    segment where some its attributes are defined similarly.

    Visually, the addressing scheme is changed from:

    CS(base):IP(offset) ----> memory byte (direct memory access)

    to:

    gdt_base + CS (where gdt_base is from %gdtr, a register containing the gdt base address and size, and 
           |         CS containts the code segment seletor, which is actually an offset from the starting address
           |         of the gdt table which usually excompasses many entries, one for each segment) 
           |
           |---> GDT entry (code segment descriptor containing base, limit, permission)
                    |
                    |
                    |---> Permitted ?
                                |
                                |Yes
                                |---> segment base + IP
                                            |
                                            |
                                            |---> memory byte (Indirect memory access with permission checking -> memory protection)

    Below is what a GDT entry looks like and all its different fields, and we choose to divide the entire
    64bit entry into 2 separate 32bit segments to introduce

                                        Field Length
    0
    segment limit (0-15)                    16
    15
    ------------------------------------------------------
    16
    segment base address (0-15)             16
    31
    ------------------------------------------------------
    0
    segment base address (16-23)            8
    7
    ------------------------------------------------------
    8
    segment type                            4

        8
        Accessed: 0 This is often used for debugging 
        and virtual memory techniques,since the CPU 
        sets the bit when it accesses the segment
        8

        9
        Readable
        9

        10
        conforming
        (if set to 0, code with a lower privilege
        may not CALL code in this segment)
        10

        11
        code
        (if set, this segment is a code sgement)
        11
    11
    ------------------------------------------------------
    12
    flages

        12
        descriptor type
        (0 = system; 1 = code or data)
        12

        13                                  4
        descriptor privilege level
        (00, 01, 10, 11) -> (0-3)
        14

        15
        segment present
        15

    15
    ------------------------------------------------------
    16
    segment limit (16-19)                   4
    19
    ------------------------------------------------------
    20
    flages

        20
        Available for 
        use by system software
        20

        20
        64bit code segment
        (IA-32e mode only)
        20

        21                                  4
        Default operation size
        (0=16bit seg, 1=32bit seg)
        21

        23
        granularity
        (if set, total seg size = limit * 4k)
        23

    13
    ------------------------------------------------------
    24
    segment base address (24-31)
    31
*/
.code16

/* null descritpor */
null_descritpor:
    .int 0
    .int 0
/* 
    kernel code segment descriptor
    base 0x00000000
    limit 0xfffff
*/
kcode_descritpor_temp:
    .word 0xffff
    .word 0x0
    .byte 0x0
    .byte 0b10011010 /* segment present = 1, privilege level = 00, descriptor type = 1(code or data), code = 1,  conforming = 0, readable = 1, AVL = 0 */
    .byte 0b11001111 /* granularity(7) is 1 so that we can access the entire 4G memory */
    .byte 0x40

/* kernel data segment descriptor */
kdata_descritpor_temp:
    .word 0xffff
    .word 0x0
    .byte 0x0
    .byte 0b10010010 /* code(3)=0 */
    .byte 0b11001111
    .byte 0x40

kcode_descritpor:
    .word 0xffff
    .word 0x0
    .byte 0x0
    .byte 0b10011010 /* segment present = 1, privilege level = 00, descriptor type = 1(code or data), code = 1,  conforming = 0, readable = 1, AVL = 0 */
    .byte 0b11001111 /* granularity(7) is 1 so that we can access the entire 4G memory */
    .byte 0x0

/* kernel data segment descriptor */
kdata_descritpor:
    .word 0xffff
    .word 0x0
    .byte 0x0
    .byte 0b10010010 /* code(3)=0 */
    .byte 0b11001111
    .byte 0x0

/* user code segment descriptor */
ucode_descritpor:
    .word 0xffff
    .word 0x0
    .byte 0x0
    .byte 0b11111010 /* privilege level(5-6) = 11(4) */
    .byte 0b11001111
    .byte 0x0

/* user data segment descriptor */
udata_descritpor:
    .word 0xffff
    .word 0x0
    .byte 0x0
    .byte 0b11110010
    .byte 0b11001111
    .byte 0x0

.set kernel_code_selector_temp, kcode_descritpor_temp - null_descritpor
.set kernel_data_selector_temp, kdata_descritpor_temp - null_descritpor
.set kernel_code_selector, kcode_descritpor - null_descritpor
.set kernel_data_selector, kdata_descritpor - null_descritpor
.set user_code_selector, ucode_descritpor - null_descritpor
.set user_data_selector, udata_descritpor - null_descritpor

.global kernel_code_selector
.global kernel_data_selector

/* GDT pointer */
gdt_ptr:
    .word gdt_ptr - null_descritpor - 1
    .long null_descritpor

.global gdt_ptr

/* Switch to protected mode! */
enter32:
    /**
     * Before the segmented model is activiated, the logical address of the gdt
     * equals its linear address. (Refer to page 91 of 421 in the i386 manual)
     * Thus, given the I386 manual states that GDTR stores the linear address 
     * of the gdt base, and that the idt descriptor base address field's been 
     * filled with the virtual (logical) address (line 216), which is 
     * 0xC0000000 + physical, the highest hex bit of 0xC should be excluded 
     * during loadind to yeild its physical address loaded into IDTR
     * 
     * This is done easily using lidtw, where the suffix - w - tells indicates 
     * a oprand size of 16 bits and to load the entire 24 bits of the diescriptor 
     * base field to IDTR, which helps exclude the highest hex bit of 0xC
     */
    lgdtw 0x38  // lgdtw -> the suffix tells the to load a 32 bit base not a 24 bit one which is indicated by suffix l if lgdtw were used
    /* Load gdt to gdtr (a register pointing to gdt in the memory) */
    mov %cr0, %eax
    or $0x1, %eax
    mov %eax, %cr0
    /* Enter the protected mode */
    ljmpl $kernel_code_selector_temp, $init32
    /* Jump to choosing the kernel code segment withe the offset instructed by the label 'init_pm' */

.extern _start
.code32
init32:
    mov $kernel_data_selector_temp, %ax
    mov %ax, %ds
    mov %ax, %ss
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    /* Set DS to choosing the kernel data segment based on which all of our later memeory access */
    mov $0xC0090000, %ebp
    mov %ebp,     %esp
    /* Set up the stack segment, preparing the running environmnet for our C code */
    mov $enter32_msg, %esi
    mov $0xC00b8000, %edi
    add $320, %edi
    call prt_str32
    jmp _start // No jumping back or returning!

prt_str32:
    mov $0x07, %ah
prt_str32_loop:
    mov (%esi), %al
    cmp $0, %al
    je prt_str32_done
    mov %ax, (%edi)
    inc %esi
    inc %edi
    inc %edi
    jmp prt_str32_loop
prt_str32_done:
    ret

enter32_msg:
    .string "Entered 32bit protected mode, stack base set to 0x90000"