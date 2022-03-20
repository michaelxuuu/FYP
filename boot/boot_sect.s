/*

    Note1. Hardware (qemu-system-i386)

        Default virtual memory size -> 128M

        BIOS implementation by qemu -> seaBIOS

        Real mode memory map (set up by BIOS within 1M) ->

        +----------------------------------------+ 0x100000 ->    1M
        |              BIOS (256K)               |
        +----------------------------------------+ 0xC0000  ->    768K
        |          Video Memory (128K)           |
        +----------------------------------------+ 0xA0000  ->    640K
        |      Extented BIOS Data Area (1K)      |
        +----------------------------------------+ 0x9FC00  ->    639K
        |               Free (638K)              |
        +----------------------------------------+ 0x7E00   ->    32256B
        |        Load Boot Sector (512B)         |
        +----------------------------------------+ 0x7C00   ->    31744B
        |               Free (≈29K)              |
        +----------------------------------------+ 0x500    ->    1280B
        |         BIOS Data Area (256B)          |
        +----------------------------------------+ 0x400    ->    1K
        |                IVT (1K)                |
        +----------------------------------------+ 0x0      ->    0B

    Note2. Real Mode Registers ->

        General registers (GR)  -> AX, BX, CX, DX

        Segment registers (SR)  -> CS (code), DS (data), ES (extra)

        Index and pointers -> SI (src index), DI (dest index), BP (stack base), SP (stack top), IP (instr pointer),

        Indicator -> EFLAGS (cpu status)

    Note3. Real Mode Addressing ->

        In Real Mode, only 20 lower bits of the address bus are usable which means the highest memory location we
        can address is 1M (2^20B). However, our registers are only 16-bit in size, which can at most contain an
        address up to 64K (2^16B). We have to use ALU to shift and add up 2 16-bit registers to obtain a valid 
        address of 20 bits considering our ALU is not limited by the 16-bit mode.

        To calculate the physical address in real mode we have to specify 2 components: base address and offset, 
        each of which is manifested by a 16-bit register. Base has to be stored in a segment register (hence the
        name) which defines the base address of a segment in memory, and the offset can be stored in a general 
        purpose register or an index register such AX, SI, IP which gives to how far we go from that base address 
        in the segment. Since the offset is a 16-bit register, the segment length is limited to under 64K (2^16B). 
        Below shows how the physical address is calculated by the ALU in real mode, where X is a hex digit.
        
        |   XXXX   | -> base << 4bits
        |  + XXXX  | -> offset
        |  ------  |
        |   XXXXX  | -> 5 * 4 = 20 bits

        EX.
        +----------------------------------------+  <--- segment range up to base addr + 64K
        |                   |                    |
        |                   |                    |
        |                   |                    |
        |                   |                    |
        +****************************************+  <--- AX (offset)
        ^                   ^                    ^
        |                   |                    |
        |                   |                    |
        |                   |                    |
        +----------------------------------------+  <--- DS (base)

        A specific example is CS:IP used in addressing instructions in Real Mode where the code has to reside in 
        the code segemnt ptointed to by the CS register while IP is the instruction pointer acting as the offset 
        for each individual instruction stored in the code segmnet.


    Note4. Location to load the bootloader and system
    
        +----------------------------------------+ 0x9FC00
        |                                        |
        |                 Loader                 | 0x90000  ->  Loader (<63K)
        |               Free (638K)              |
        |                 System                 | 0x10000  ->  System (<512K)
        |                                        |
        +----------------------------------------+ 0x7E00
        |        Load Boot Sector (512B)         |
        +----------------------------------------+ 0x7C00

    Note5. seaBIOS and IDE (integrated drive electronics) support for CHS (cylinder-head-sector) addressing mode:

        seaBIOS disk load routine -> int 0x13, ah = 0x02

        IDE (Integrated Drive Electronics) is an electronic interface standard that defines the connection between a bus 
        on a computer's motherboard and the computer's disk storage devices (ref'ed from https://searchstorage.techtarget.com/definition/IDE#:~:text=IDE%20(Integrated%20Drive%20Electronics)%20is,the%20computer's%20disk%20storage%20devices.&text=After%20ANSI%20standardized%20the%20technology,started%20to%20be%20used%20interchangeably)

        seaBIOS and IDE spec collectively give limitaions to disk accessing in CHS mode ->

                                        BIOS        IDE          Combined

            Max. sectors per track:      63         255             63  (63 still true today for virtual CHS mode)
            Max. heads:                 256         16              16  (This is why if you real above 16 sectors using BIOS routine BIOS woundn't throw an error but the read would fail silently)
            Max. Cylinders              1024        65536           1024

            (ref'd from http://web.inter.nl.net/hcc/J.Steunebrink/bioslim.htm)

            BIOS limitations are due to the bits it assigned to each parameter (C,H,S)

        Note that this IDE spec is outdated viewed from today but it was true for the days of i386 mcahines, and qemu 
        i386 system emulates exactly that that (see "DESCRIPTION" section at https://manpages.debian.org/stretch/qemu-system-x86/qemu-system-i386.1.en.html)

        Qemu reads your files as a virtul disk and treat it as as if in LBA mode (all blocks are linearly organized)
        and if you use CHS mode, it is actually simulated by qemu. So, you don't have to worry so much about the actual
        disk geometry since it was just a file (virtual disk) with all blocks are laying lienarly one after another, and 
        the so-called CHS is just a fraud created by qemu. Therefore, if you are attempted to use CHS mode in BIOS disk
        access routine, you can just view your disk as if it only has 63 sectors, 16 heads, and 1024 cylinders, while remebering
        tightly that this CHS mode doesn't actually exit but is faked and virtual.

        To linearly read the virtual disk in the simulated CHS mode, qemu, starting from head 0, firstly reads each track 
        in the first cylinder (C = 0) till head 16 is reached, and then it moves to the next cylinder (C = 1), and it repeats
        the above process on and on till the end of the loop.
*/
.code16
.global init16
.text

init16:
    mov $0, %ah /* func -> set video mode */
    mov $3, %al /* param -> 80x25 text mode */
    int $0x10
    /* Reset video mode to clear some qemu start-up messages */
    mov $0x9000, %sp
    mov %sp, %bp
    /* Set up the stack */
    push %dx
    /* Save the boot drive that BIOS has put in dl for us */
    mov $0x07c0, %ax    
    mov %ax, %ds
    mov $0x9000, %ax
    mov %ax, %es
    sub %si, %si
    sub %di, %di
    mov $256, %cx
    rep movsw
    ljmp $0x9000, $go16
    /*
     The above code moves the code from 0x7c00 where the BIOS start-up routine loads our boot loader code to 0x90000. 
     CS has initially pointed to 0x7c00 and started fetching instructions there, but it cannot be used to move data, 
     so we need another register to point to it.
     (ds:si -> es:di)
    */

go16:
    mov $0x9000, %ax
    mov %ax, %ds
    /*
        The above code is necessary since as previously stated in the real mode the addressing is done relative to segment registers. 
        For instruction fetching, we have seen the CS:IP scheme. Here, for loading data, the segment register is changed to DS.
        For example, if we want to move the address of a label to a register (%si) and then use that register as the base address
        to load data from memory to another register (%al), we would write the following code:

        mov $label, %si
        mov (%si), %al

        However, what happens in fact is by 'mov (%si), %al' the data is fetched from the relative address: DS:SI instead of
        the the absolute address of the label calculated in compile time. Since we have moved the entire boot sector from
        7c00 to 90000, and jumped to there and started executing, so we also need to update the DS register based on which 
        we will be laoding data from the memory.

        Suprisingly, deleting the code of updating DS does not make our kernel crash for an error, and I'll leave you to think 
        of the reason why.
    */
    
/* Load the system at 0x10000 */
    mov $disk_msg, %si
    call prt_str16
    /* Print the loading message */
/* 
    Here we assume the kernel size is 512K in exact, though our kernel at this stage is much smaller than that in size.
    Therefore, we have to calculate how many cylinders and tracks we have to read. With each sector being 512 bytes, we
    need to read 1024 sectors at total. Accroding to the hardware and BIOS CHS support, each cylinder has 16 heads and each
    track has 63 tracks. So, each cylinder contains 16 * 63 which is 1008 setcors, and whereby we need to read 1 cylinder 
    plus 16 more sectors. Additionally, since the first sector is the boot sector is the boot setcor which is not a part of
    the kernel code, so we actually need to read the entire first cylinder and the 17 more sectors on the first track of the
    second layer. We decide to read one track at a time and loop to read off all 1024 requested sectors.

    However, we have to simultaneously consider the 64K boundary that we should not pass (remembering that the BIOS write to
    EX:BS where BS is 16-bit and addresses 2^16 = 64K memeory space). 64K is 64 * 1024 / 512 = 128 sectors. Hence, whenever
    128 sectors are read, we update EX register to let the BIOS write to the next 64K area.

    Initially, we read the sectors excluding the boot sectors from the first track of the first cylinder, which are 62 sectors,
    and then we are ready to read the next track which is 63 sectors, and at total, we have had 125 sectors, which is 3 sectors
    less to the 128 sector (64K) limit. We then go on read off the next track, but next time, we can not read an entire track since
    the 128 sector boundary is reached, menaing that we have to first read 3 setcors to get to 128 sectors and then adjust EX
    to pointing to the start of the next 64K segment...

    In that way, before reading each track, we have to check if there are sectors having been read already and do not need
    to be read. After reading each track, we have to check if the boundary is able to take another track and if it cannot
    how many sector it will have to read to get to 64K and calculate the remaining sectors in the current track that will be
    read in the next loop.

    Var: #cylinder                      /0                          -> current cylinder
    Var: #head                          /0                          -> current head
    Var: #sector                        /0                          -> current sectors read
    Var: #seg_free                      /128                        -> number of sectors the current ES:BX can take
    Var: #sector                        /62                         -> sectors to be read from a track

    1. Read #sector sectors from the current head
    2. Sub #sector from #seg_free
    3. Add #sector to #setcor
    3. Compare #head with 16:                                   -> Move to next head
        If #head is greater than 16:
            Add 1 to #cylinder
            Make #head 0
        else:
            Add 1 to #head
    4. Compare #seg_free with 63,                             -> test if the remianing space of the current segment can contain another entire track
        If #seg_free < 63:                                    -> if the space left is smaller than 63 sectors
            Read #seg_free sectors from the current head      -> read the sectors to fill up the 128 sector segment
            Set #sector to 63 - #sector             -> next time we read only the remianing sectors from the current head
            Add #seg_free to #sector                          -> update the number of total sectors read
            Set #seg_free to 128                              -> move to next 128 sector segment
            Set EX to EX + 1                                    -> update EX
            Set BX to 0                                         -> update BX
        Else 
            Set #sector to 63
            Set bx to bx + 63 * 512 (32256)
    
*/
    pop %dx
    mov $0x1000, %ax
    mov %ax, %es
    mov $0, %bx
    /* Initialize ES:BX */
disk_loop:
/* Read track */
    mov cylinder, %ch
    /* Set cylinder */
    mov head, %dh
    /* Set head */
    mov $64, %cl
    sub sector, %cl
    /* Set starting sector; cl = 64(cl) - 'sector' */
    mov sector, %al
    /* Set the number of sectors */
    mov $0x02, %ah
    int $0x13
    /* 
        Read from disk (emulator guarantees error-free but errors do occur 
        when the virtual disk created by dd is not large enough) 
    */
    jc disk_err
    sub sector, %al
    jnz disk_err
    /* Error ? */
    mov sector, %ax
    shl $9, %ax
    add %ax, %bx
    /* Adjust the segment offset BX by the number sectors read bx = bx + 512 * sector !!!!!! Important to be exactly here !!!!!! */
    mov sector, %ax
    addw %ax, ttl_sector
    /* Update the number of total sectors read */
    mov sector, %ax
    subw %ax, seg_free
    /* Decrease the free size in current 64K segment by sectors read on the current track */
    addw $1, head
    /* Move to the next head in the currrent cylinder */
    cmpw $16, head      /* The immediate must be first oprand */
    /* Test if we have done with the current cylinder (head > 15). Note that the head count starts with 0 */
    jl test_seg
    /* Skip the code that increments 'cylinder' if head <= 16 meaning that the current cylinder is not yet finished */
    addw $1, cylinder
    movw $0, head
    /* Move to next cylinder */

test_seg:
    cmpw $63, seg_free
    /* Test if current 64K segment could not hold another 63 sectors */
    jl pre_read
    /* If the free space is less than 63 setcors, we have to jump to the part
    that reads certain sectors from the new track to fill up the current segment 
    and then update the segment register EX and offset register BX and the code
    also update 'sector' variable by subtracting the sectors it read for filling
    the current sectors, so that the next iteration will only read the rest of the
    sectors on the new track */
    movw $63, sector
    /* Next time we read the entire track */
    jmp disk_loop_cond
    /* Test the looping condition */

pre_read:
    mov head, %dh
    /* Set head which may have been updated */
    mov cylinder, %ch
    /* Set cylinder which may have been updated */
    mov $1, %cl
    /* Set the strating sector */
    mov seg_free, %al
    /* Read 'seg_free' sectors to fill up the current 64K sector */
    mov $0x02, %ah
    int $0x13
    /* Read it ! */
    jc disk_err
    sub seg_free, %al
    jne disk_err
    /* Error ? */
    mov $63, %ax
    sub seg_free, %ax
    mov %ax, sector
    /* Set #sector to 63 - #seg_free, so that the next time we will only have 
    to read the remianing sectors in the track */
    mov seg_free, %ax
    addw %ax, ttl_sector
    /* Update the number of total sectors read */
    movw $128, seg_free
    /* Reset the free size counter to 128 for the new segment */
    mov %es, %ax
    add $0x1000, %ax
    mov %ax, %es
    /* Move EX to pointing to the base of next 128 sector segment */
    sub %bx, %bx
    /* Zero BX */

/* Looping condition */
disk_loop_cond:
    cmpw $1024, ttl_sector
    jl disk_loop

/* Relocate the system from 0x10000 to 0x0 (ds:si -> es:di)*/
    mov $relct_msg, %si
    call prt_nxline16
    /* 
        For the last time we use BIOS interrupt to declare we are moving the system from 0x10000 to 0x0 after which the BIOS
        routines will no longer be useful since we will have overwritten the IVT residing at the lowest 1K memory space
     */
    mov $0x0, %ax
    cld             
    /* Foward (downward) direction */

relct_loop:
    mov %ax, %es
    /* Destination segment */
    add $0x1000, %ax
    /* Next 64K segment */
    cmp $0x9000, %ax
    /* Last segment ? */
    je sys_ready
    /* End looping if last */
    mov %ax, %ds
    /* 
        Source segment (= Dest seg + 0x10000)

            src         dest
          0x1000       0x0000
          0x2000       0x1000
          0x3000       0x2000
                  ...
    */
    sub %si, %si
    sub %di, %di
    mov $0x8000, %cx
    /* 64K bytes = 32K words = 2^5 * 2^10 = 2^15 = 2^3 * 2^12 = 8 * 2^4^3 = 0x8000 */
    rep movsw
    jmp relct_loop

sys_ready:
    cli
    /* BIOS's IVT has been overwritten so intrrupts are now useless, thus we trun it off */
    mov $0x0, %ax
    mov %ax, %ds
    ljmp $0x0, $0x3e
    /* Do a long jump to 0x0:46 to execute our system code. But why 46? See kernel_entry.s and compile it
    and use 'i386-elf-objdump -t' to inspect the symbol table to try to find symbol 'pm_init' */


disk_err:
    mov $disk_err_msg, %si
    call prt_nxline16
    jmp .

sysr_msg: 
    .string "Sys ready!"

/* Resuable routines */
prt_str16:
    /* Routine to print a string, 
        %si is expected to contain the starting address of the string */
    mov $0x0e, %ah
    /* Scrolling teletype BIOS routine */
prt_str16_loop:
    mov (%si), %al
    /* Load one character from (%si) to al as the parameter for the BIOS routine */
    cmp $0, %al
    /* Test for zero */
    je prt_str16_done
    /* Done if zero is reached */
    int $0x10
    /* Print one character */
    add $0x1, %si
    /* Update %si to point to the next character */
    jmp prt_str16_loop
    /* Loop back */
prt_str16_done:
    ret

prt_nxline16:
    mov $3, %ah /* func -> get cursor position */
    mov $0, %bh /* param -> video page 0 */
    int $0x10
    /* Get cursor position dl=x, hd=y */
    inc %dh         
    /* Move cursor down a row */
    xor %dl, %dl    
    /* move cursor to the begining of row */
    mov $2, %ah /* func -> set text mode sursor position */
    mov $0, %bh /* param -> video page 0 */
    int $0x10
    /* Repositing cursor by dl, and hd */
    call prt_str16
    ret

cylinder:
    .word 0

head:
    .word 0

sector:
    .word 62

ttl_sector:
    .word 0

seg_free:
    .word 128

disk_msg:
    .string "Loading system at 0x10000..." /* Must be put here not right after 'end_hello' or the data
                                            will actually be wronly interpreted as code and yields undefined behavior */
disk_err_msg:
    .string "Disk error..."

relct_msg:
    .string "Moving system from 0x10000 to 0x0..."

enter32_msg:
    .string "Setting up GDT and entering 32-bit protected mode..."

    .fill 510-(.-init16), 1, 0    /* add zeroes to make it 510 bytes long */
    .word 0xaa55                /* magic bytes that tell BIOS that this is bootable */