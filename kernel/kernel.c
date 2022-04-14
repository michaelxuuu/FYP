#include "../drivers/screen.h"
#include "../cpu/comn_hdlr.h"
#include "../drivers/kbd.h"
#include "../mem/pm_mngr.h"
#include "../mem/vm_mngr.h"
#include "../drivers/hd.h"
#include "ksbrk.h"
#include "kmalloc.h"
#include "kprintf.h"
#include "../fs/iocache.h"

#define db(label) \
		__asm__ volatile (label)

int _start() {
	clear_screen();
	pm_mngr_init();
	vm_mngr_init();
	interrupt_init();
	keyboard_init();
	iocache_init();
	for(;;);
}

