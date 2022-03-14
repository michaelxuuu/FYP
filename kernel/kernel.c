#include "./../drivers/screen.h"
#include "./../cpu/comn_hdlr.h"
#include "./../drivers/kbd.h"
#include "./../mem/pm_mngr.h"
#include "./../mem/vm_mngr.h"

void hold_for(int i) {
	for (; i > 0; --i);
}

int _start() {
	clear_screen();
	install_idt();
	register_keyboard_handler();
	__asm__ volatile("sti");
	printf("Here comes the fault...\n");
	__asm__ volatile("int $1");
	printf("Hahaha, Piggy is back!\n");
	printf("asdas%f\nasdas",0.11);
	pm_mngr_init();
	// uint32_t addr_test = pm_mngr_alloc_block();
	// pm_mngr_free_block(524288);
	// vm_mngr_enable_paging(1);
	vm_mngr_init();
	printf("asdas%f\nasdas",0.11);
	for(;;);
}