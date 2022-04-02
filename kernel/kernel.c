#include "./../drivers/screen.h"
#include "./../cpu/comn_hdlr.h"
#include "./../drivers/kbd.h"
#include "./../mem/pm_mngr.h"
#include "./../mem/vm_mngr.h"
#include "./../drivers/hd.h"

int _start() {
	clear_screen();
	pm_mngr_init();
	vm_mngr_init();
	__asm__
	(
		"pop %ebp;"
		"mov $0xffc00000, %esp;"
		"mov %esp, %ebp;"
		"push %ebp;"
	);
	interrupt_init();
	keyboard_init();
	for(;;);
}

