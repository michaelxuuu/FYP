#ifndef PIC_H
#define PIC_H

#include "../include/type.h"
#include "../drivers/low_level.h"

/*
    Master PIC IP base address is 0x20 (command port 0x20, data port 0x21)
    Slave PIC IO base address is 0xA0 (command port 0xA0, data port 0xA1)
*/

#define MASTER_CMD 0x20
#define MASTER_DATA 0x21
#define SLAVE_CMD 0xA0
#define SLAVE_DATA 0xA1

#define PIC_EOI 0x20		/* End-of-interrupt command word */


/*
    Notes:
        1. To initialize PIC, CPU sends ICWS (initialization command words -> 2 byte in size...) to the IO port of the PIC
        2. Master and slave PIC's are initilized respectively by the same sequence
        3. Initialization sequence: wirte ICW1 to ICW4 successively (ICW1 to cmd port, the rest to data port)
            - ICW1 -> Starts the initialization sequence
            - ICW2 -> PIC vector offset (may not start from 0...  Consider intrrupt 0 to 31 being occupied already... So we remap...)
            - ICW3 -> Indicate a master PIC which IR line is conneted to a slave PIC
            - ICW4 -> Indicate the cpu type : 8086, 8085, etc. or others that the 8259A PIC supports
        
    For more details, check out the manual at https://pdos.csail.mit.edu/6.828/2005/readings/hardware/8259A.pdf
    I also found this tutorial helpful and put it here for reference: http://www.brokenthorn.com/Resources/OSDevPic.html
*/

/* Define some useful ICW options (options will not be used aren't included) */

#define ICW1_ICW4	      0b000000001		/* ICW4 needed in the initialization sequence (not needed if not set) */
#define ICW1_SINGLE	      0b000000010		/* Single mode (cascade mode if not set) */
#define ICW1_INTERVAL4	  0b000000100		/* Call address interval 4 (8) */
#define ICW1_LEVEL	      0b000001000		/* Level triggered mode (edge triggered mode if not set) */
#define ICW1_INIT	      0b000010000		/* This bit set indicates an ICW1 and reqests to start initialization */
   
#define ICW4_8086	      0b000000001		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	      0b000000010		/* Auto (normal) EOI */

/* i386 specific constants */

#define ICW2_M_OFFSET 0x20     /* 32 in decimal -> master vector offset to 32 */
#define ICW2_S_OFFSET 0x28     /* 40 in decimal -> slave vector offset to 40 (7 after the master's) */
#define ICW3_M 0x04            /* sent to Master PIC: there is a slave PIC at IRQ2 (0000 0100) */
#define ICW3_S 0x02            /* sent to Slave PIC: its cascade identity (0000 0010) */

void pic_init();

void pic_send_EOI(uint32_t irq_n);

#endif