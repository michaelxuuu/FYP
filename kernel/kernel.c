#include "../drivers/screen.h"
#include "../cpu/comn_hdlr.h"
#include "../drivers/kbd.h"
#include "../drivers/timer.h"
#include "../mem/pm_mngr.h"
#include "../mem/vm_mngr.h"
#include "../drivers/hd.h"
#include "ksbrk.h"
#include "kmalloc.h"
#include "kprintf.h"
#include "../fs/iocache.h"
#include "../fs/fs.h"
#include "../user/tss.h"
#include "../user/proc.h"
#include "../user/syscall.h"


#define db(label) \
		__asm__ volatile (label)

int _start() {
	clear_screen();
	pm_mngr_init();
	vm_mngr_init();
	interrupt_init();
	timer_init(50);
	keyboard_init();
	iocache_init();
	fs_init();
	// fs_read();
	syscall_init();
	proc_init();
	for(;;);
}