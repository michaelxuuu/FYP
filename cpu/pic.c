#include "pic.h"

void pic_init() 
{
    // ICW1
    port_byte_out(MASTER_CMD, ICW1_INIT | ICW1_ICW4); // Start initializing master whose initialization sequence will include ICW4
    port_byte_out(SLAVE_CMD, ICW1_INIT | ICW1_ICW4);  // Start initializing master whose initialization sequence will include ICW4
    // ICW2 (vector offset)
    port_byte_out(MASTER_DATA, ICW2_M_OFFSET);
    port_byte_out(SLAVE_DATA, ICW2_S_OFFSET);
    // ICW3 (configure cascade mode on both master and slave PICs)
    port_byte_out(MASTER_DATA, ICW3_M);
    port_byte_out(SLAVE_DATA, ICW3_S);
    // ICW4 (specify the cpu type)
    port_byte_out(MASTER_DATA, ICW4_8086);
    port_byte_out(SLAVE_DATA, ICW4_8086);
    // Enable all IRQ lines on master and slave PICs
    port_byte_out(MASTER_DATA, 0x0);
    port_byte_out(SLAVE_DATA, 0x0);
}

void pic_send_EOI(uint32_t irq_n)
{
    if (irq_n < 8)
        port_byte_out(SLAVE_CMD, PIC_EOI);
    port_byte_out(MASTER_CMD, PIC_EOI);
}