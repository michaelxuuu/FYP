#include "./../drivers/screen.h"
#include "./../cpu/comn_hdlr.h"
#include "./../drivers/kbd.h"
#include "./../mem/pm_mngr.h"
#include "./../mem/vm_mngr.h"

int _start() {
	clear_screen();
	pm_mngr_init();
	vm_mngr_init();
	install_idt();
	__asm__ ("int $1;"); // Successfully handled
	__asm__ ("jmp 0x0;"); // Page Fault
	for(;;);
}

