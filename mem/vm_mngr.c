#include "vm_mngr.h"

int 
va_get_dir_index(virt_addr va) {
    // virt_addr is uint32_t, so this shift is performed on an unsigned value and so to be logical
    return va >> 22;
}

int 
va_get_page_index(virt_addr va) {
    return va >> 12 & 0x3FF; // Must mask off the dir index part
}

int 
va_get_page_offset(virt_addr va) {
    return va & 0xFFF; // Mask off the index part
}

void 
page_add_attrib(physical_addr* ptr_to_pte, int attrib) {
    *ptr_to_pte |= attrib;
}


void 
page_del_attrib(physical_addr* ptr_to_pte, int attrib) {
    *ptr_to_pte &= ~attrib;
}


void 
page_install_frame_addr(physical_addr* ptr_to_pte, physical_addr frame_addr) {
    // Clear the possibly existing frame address and install the new frame address
    *ptr_to_pte = (*ptr_to_pte & ~PAGE_FRAME_ADDR_MASK) | ((frame_addr / BLOCK_SIZE) << 12);
}

int 
page_is_present(pte e) {
    return e & PAGE_PRESENT;
}


int 
page_is_user(pte e) {
    return e & PAGE_USER;
}


int 
page_is_accessed(pte e) {
    return e & PAGE_ACCESSED;
}


int
page_is_dirty(pte e) {
    return e & PAGE_DIRTY;
}


physical_addr 
page_get_frame_addr(pte e) {
    // virt_addr is uint32_t, so this shift is performed on an unsigned value and so to be logical
    return (e >> 12) * BLOCK_SIZE;
}

physical_addr cur_pd_addr;

void 
vm_mngr_load_pd(physical_addr pd_addr) {
    __asm__
    (
        "mov %0, %%eax;"
        "mov %%eax, %%cr3;"
        :: "r" (pd_addr)
    );
}

physical_addr
vm_mngr_alloc_frame(pte* ptr_to_pte) {
    // Aquire a free block from our physical memory manager
    physical_addr frame_addr = pm_mngr_alloc_block();

    // Physical memory exhausted, allocation failure
    if (!frame_addr)
        return 0;

    // Install the PTE frame address field with the physical address
    page_install_frame_addr(ptr_to_pte, frame_addr);

    // Set the page to RESENT to denote a successful page allocation, other attributes are not necessarily set here
    page_add_attrib(ptr_to_pte, PAGE_PRESENT);

    return frame_addr;
}

void
vm_mngr_free_frame(pte* ptr_to_pte) {
    // Get the frame address
    physical_addr frame_addr = page_get_frame_addr(*ptr_to_pte);
    
    // Do nothing if the page is unmapped, namely its frame address is 0x0
    // Otherwise, the kernel area of the physical memory shall be in grave danger
    if (frame_addr)
        pm_mngr_free_block(frame_addr);
    
    // Clear the present bit to indicate the frame has been unallocated and the page is unmapped
    page_del_attrib(ptr_to_pte, PAGE_PRESENT);
}

void
vm_mngr_map(physical_addr pa, virt_addr va) {

    /**
     * The idea is simple:
     * 1. Obtain the PTE based on the given virtual address (VA)
     *      1.1 Get (a pointer to) the PD
     *      1.2 Get (a pointer to) the PDE
     *      1.3 Get (a pointer to) the PT
     *      1.4 get (a pointer to) the PTE
     * 2. Assign the given physical address to the frame address field of the PTE
     */

    // Pointer to the page directory
    physical_addr* ptr_to_pd = (physical_addr*) cur_pd_addr;

    // Pointer to the page directory entry, which points to the page table
    pte* ptr_to_pde = ptr_to_pd + va_get_dir_index(va); // A page directoiry entry is in essence a page table entry (PTE)

    // Allocate a frame for the 'page table' page if PDE PRESENT bit unset
    if (!page_is_present(*ptr_to_pde)) {
        physical_addr* frame_addr = (physical_addr*)vm_mngr_alloc_frame(ptr_to_pde);
        if (!frame_addr)
            return;
        // Clear the page table to all 0's
        mem_set(frame_addr, 0, 4096);
    }

    // Pointer to the page table
    physical_addr* ptr_to_pt = (physical_addr*)page_get_frame_addr(*ptr_to_pde);

    // Pointer to the page table entry
    pte* ptr_to_pte = ptr_to_pt + va_get_page_index(va);

    // Mark the page as PRESENT and assign it with the physcial address passed in
    page_install_frame_addr(ptr_to_pte, pa);
    page_add_attrib(ptr_to_pte, PAGE_PRESENT | PAGE_WRITABLE); // Kernel Page is automatically and implicitly implied by not setting PAGE_USER
}

void 
vm_mngr_init() {
    /**
     * 1. Identity map the kernel so that the execution of the current code won't be affacted when enabling the paging
     * 2. Map the kernel to 3GB virtual to make a higher half kernel
     */

    // Allocate memory for a default page directory table
    cur_pd_addr = pm_mngr_alloc_block();
    
    // Load the address of the PD to the PDBR
    vm_mngr_load_pd(cur_pd_addr);

    // Clear the directory table
    mem_set((physical_addr*)cur_pd_addr, 0, 4096);

    // Identity map the kernel (also the lower 4M physical memory)
    for (int pa = 0x0, va = 0x0, ct = 0; ct < 1024; ct++, pa += 4096, va += 4096)
        vm_mngr_map(pa, va);

    // Map the kernel to 3G virtual
    for (int pa = 0x0, va = 0xC0000000, ct = 0; ct < 1024; ct++, pa += 4096, va += 4096)
        vm_mngr_map(pa, va);
    
    vm_mngr_enable_paging(1);
}

void 
vm_mngr_enable_paging(int enable) {
    /**
     * Stack:
     *  esp + 8     argumnet       (pushed by 'pushl $1' - generated by the compiler, in the caller scope)
     *  esp + 4     return address (pushed by the 'call' instr)
     *  esp         caller's ebp   (pushed by 'push ebp' - generated by the compiler, in the callee scope)
     */
    __asm__
    (
        "mov %cr0, %eax;"
        "cmp $1, 8(%esp);"
        "je paging_en;"
        "and $0x7FFFFFFF, %eax;"
        "mov %eax, %cr0;"
        "jmp done;"
        "paging_en:;"
        "or $0x80000000, %eax;"
        "mov %eax, %cr0;"
        "done:;"
    );
}
