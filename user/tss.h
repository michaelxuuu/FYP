#ifndef TSS_H
#define TSS_H

#include"../include/type.h"

#include"../Lib/util.h"

/* External veriables defined in kernel_entry.s */
extern uint32_t kcode_selector;
extern uint32_t kdata_selector;
extern uint32_t tss_descriptor;
extern uint32_t tss_selector;

void goto_user();

#endif