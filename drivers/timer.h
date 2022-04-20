#ifndef TIMER_H
#define TIMER_H

#include "../cpu/comn_hdlr.h"
#include "../cpu/idt.h"
#include "../include/type.h"
#include "../user/proc.h"

void timer_init(uint32_t freq);

#endif