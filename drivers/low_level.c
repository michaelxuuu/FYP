#include "low_level.h"
// C inline assambly:
// Format: instr : output : input
// Note: instr is written in intel format,
// where the destination is placed on the right side

// char is 1 byte
unsigned char port_byte_in(unsigned short port) {
    unsigned char result;
    __asm__("in %%dx, %%al" : "=a"(result) : "d"(port));
    // = is to assign varibale
    // Variables parenthesized are "related" to the registers next to them
    return result;
}

void port_byte_out(unsigned short port, unsigned short data) {
    __asm__("out %%al, %%dx" : : "d"(port) , "a"(data));
}

// short is 2 byte (1 word)
unsigned short port_word_in(unsigned short port)
{
    unsigned short result;
    __asm__("in %%dx, %%al" : "=a"(result) : "d"(port));
    return result;
}

void port_word_out(unsigned short port, unsigned short data)
{
    __asm__("out %%al, %%dx" : : "d"(port),  "a"(data));
}







