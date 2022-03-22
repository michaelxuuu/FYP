#include "kbd.h"

void keyboard_callback(irq_reg_info *r)
{
    printf("Keyboard!\n");
    uint8_t scancode = port_byte_in(0x60);
    printf("%d\n", (uint32_t)scancode);
}

void keyboard_init()
{
    register_handler(33, keyboard_callback);
}