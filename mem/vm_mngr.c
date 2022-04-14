#include "vm_mngr.h"

int 
va_get_dir_index(uint32_t va) {
    // uint32_t is uint32_t, so this shift is performed on an unsigned value and so to be logical
    return va >> 22;
}

int 
va_get_page_index(uint32_t va) {
    return va >> 12 & 0x3FF; // Must mask off the dir index part
}

int 
va_get_page_offset(uint32_t va) {
    return va & 0xFFF; // Mask off the index part
}

void 
page_add_attrib(uint32_t* ptr_to_pte, int attrib) {
    *ptr_to_pte |= attrib;
}


void 
page_del_attrib(uint32_t* ptr_to_pte, int attrib) {
    *ptr_to_pte &= ~attrib;
}


void 
page_install_frame_addr(uint32_t* ptr_to_pte, uint32_t frame_addr) {
    // Clear the possibly existing frame address and install the new frame address
    *ptr_to_pte = (*ptr_to_pte & ~PAGE_FRAME_ADDR_MASK) | ((frame_addr / BLOCK_SIZE) << 12);
}

int 
page_is_present(uint32_t pte) {
    return pte & PAGE_PRESENT;
}


int 
page_is_user(uint32_t pte) {
    return pte & PAGE_USER;
}


int 
page_is_accessed(uint32_t pte) {
    return pte & PAGE_ACCESSED;
}


int
page_is_dirty(uint32_t pte) {
    return pte & PAGE_DIRTY;
}


uint32_t 
page_get_frame_addr(uint32_t e) {
    // uint32_t is uint32_t, so this shift is performed on an unsigned value and so to be logical
    return (e >> 12) * BLOCK_SIZE;
}

void 
vm_mngr_load_pd(uint32_t pd_physical_addr) {
    __asm__
    (
        "mov %0, %%eax;"
        "mov %%eax, %%cr3;"
        :: "r" (pd_physical_addr)
    );
}

uint32_t
vm_mngr_alloc_frame(uint32_t* ptr_to_pte) {
    // Aquire a free block from our physical memory manager
    uint32_t frame_addr = pm_mngr_alloc_block();

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
vm_mngr_free_frame(uint32_t* ptr_to_pte) {
    // Get the frame address
    uint32_t frame_addr = page_get_frame_addr(*ptr_to_pte);
    
    // Do nothing if the page is unmapped, namely its frame address is 0x0
    // Otherwise, the kernel area of the physical memory would be in grave danger
    if (frame_addr)
        pm_mngr_free_block(frame_addr);
    
    // Clear the present bit to indicate the frame has been unallocated and the page is unmapped
    page_del_attrib(ptr_to_pte, PAGE_PRESENT);
}

uint32_t* cur_pd_addr;

void
vm_mngr_lower_kernel_map(uint32_t va, uint32_t pa)
{
    // Pointer to the current page directory
    uint32_t *pd = cur_pd_addr;

    // Pointer to the page directory entry
    uint32_t *pde = pd + va_get_dir_index(va);

    // Create a page table if not present bit unset
    if (!page_is_present(*pde))
        mem_set((char*)(vm_mngr_alloc_frame(pde) + 0xC0000000), 0, BLOCK_SIZE); // Newly created page table must be cleared first

    // Pointer to the page table (page_get_frame_addr returns the physical address, need to convert it to logical for memory access)
    uint32_t *pt = (uint32_t*)(page_get_frame_addr(*pde) + 0xC0000000);

    // Pointer to the page table entry
    uint32_t *pte = pt + va_get_page_index(va);

    // Install the page table entry
    page_install_frame_addr(pte, pa);
    page_add_attrib(pte, PAGE_PRESENT | PAGE_WRITABLE);
}

#define KERNEL_STACK_BASE (0xFFC00000 - 1)
#define KERNEL_STACK_LIMI (KERNEL_STACK_BASE - 0x400000 + 1)

void
vm_mngr_init() {
    /**
     * 1. Identity map the kernel so that the execution of the current code won't be affacted when enabling the paging
     * 2. Map the kernel to 3GB virtual to make a higher half kernel
     * 3. Set up the kernel address space
     */

    // Allocate 4M memory for Kernel Stack (fixed size, frames not meant to ever be freed at any time)
    for (int i = 0; i < 1024; i++) {
        mem_set((char*)(pm_mngr_alloc_block() + 0xC0000000), 0, BLOCK_SIZE);
    }

    // Allocate 4K memory for the utility page
    mem_set((char*)(pm_mngr_alloc_block() + 0xC0000000), 0, BLOCK_SIZE);

    // Create a page directory table
    cur_pd_addr = (uint32_t*)(pm_mngr_alloc_block() + 0xC0000000);
    mem_set((char*)cur_pd_addr, 0, BLOCK_SIZE);

    // Load the PHYSICAL address of the page directory to the PDBR (HENCE minus 0xC0000000)
    vm_mngr_load_pd((uint32_t)cur_pd_addr - 0xC0000000);

    // Identity map the first 1M
    for (uint32_t pa = 0x0; pa < 0x100000; pa += BLOCK_SIZE)
        vm_mngr_lower_kernel_map(pa, pa); // physical addr = virtual addr in identity mapping

    // Map the first 1M to 3GB virtual (1 new page table created)
    for (uint32_t va = 0xC0000000, pa = 0x0; pa < 0x100000; pa += BLOCK_SIZE, va += BLOCK_SIZE)
        vm_mngr_lower_kernel_map(va, pa);

    // Map the kernel stack to 0xffc00000 (the last page of the virtual address space)
    for (uint32_t va = KERNEL_STACK_LIMI, pa = 0x100000; pa < 0x500000; pa += BLOCK_SIZE, va += BLOCK_SIZE)
        vm_mngr_lower_kernel_map(va, pa);

    // Map the utility page to 0xC0100000
    vm_mngr_lower_kernel_map(0xC0100000, 0x500000);
    // Link the utility page table to the last PDE in the directory table
    page_install_frame_addr(cur_pd_addr + 1023, 0x500000);
    page_add_attrib(cur_pd_addr + 1023, PAGE_PRESENT | PAGE_WRITABLE);

    // Map the kernel page directory to 0xC0101000
    vm_mngr_lower_kernel_map(0xC0101000, 0x501000);

    // Enable paging
     __asm__ volatile
    (
        // Enable paging
        "mov %cr0, %eax;"
        "or $0x80000000, %eax;"
        "mov %eax, %cr0;"

        // Jump to higher half kernel
        /**
         * With paging enabled and the GDT base address field being 0, the logical address of the gdt base
         * equals its linear address. (Refer to page 91 of 421 in the i386 manual) Thus, given the I386 manual 
         * states that GDTR stores the linear address of the gdt base, and that the gdt descriptor base address 
         * field's been filled with the virtual (logical) address (see line 216 in kernel_entry.s), we should 
         * directly load the entire gdt descriptor base address of 32 bits into GDTR.
         * 
         * This is done easily using lgdtl, where the suffix - l - tells indicates a oprand size of 32 bits 
         * and to load the entire 32-bit the diescriptor base field to GDTR
         */
        "lgdtl gdt_ptr;" // lgdtl -> the suffix tells the to load a 32 bit base not a 24 bit one which is indicated by suffix w if lgdtw were used
        "ljmpl $kernel_code_selector, $switch_gdt;"
        "switch_gdt:;"
        "mov $kernel_data_selector, %ax;"
        "mov %ax, %ds;"
        "mov %ax, %ss;"
        "mov %ax, %es;"
        "mov %ax, %fs;"
        "mov %ax, %gs;"
    );

/* Copy the kernel stack from 0x90000 to 0xFFC00000 */

    // Get the value that %esp and %ebp holds
    uint32_t ebp, esp;
    __asm__ volatile
    (
        "mov %%ebp, %0;"
        "mov %%esp, %1;"
        : "=r" (ebp), "=r" (esp)
    );

    // Copy the memory from 0x90000 to %esp (%esp <= %ebp) to the memory region starts from 0xFFFFFFFF downwards
    for (int i = 0, j = 0; i < 0xC0090000 - esp; i++, j--)
        ((uint8_t*)KERNEL_STACK_BASE)[j] = ((uint8_t*)0xC0090000)[j];

    // Have %esp and %ebp point to the corrsponding addresses in the mapped stack region
    ebp = KERNEL_STACK_BASE - (0xC0090000 - ebp);
    esp = KERNEL_STACK_BASE - (0xC0090000 - esp);
    __asm__ volatile
    (
        "mov %0, %%esp;"
        "mov %1, %%ebp;"
        :: "r" (esp), "r" (ebp)
    );

    // Update the current page diretory pointer (have it point to the mapped PD in 3G virtual)
    cur_pd_addr = (uint32_t*)0xC0101000;

/* Discard the lower kernel */

    // Delete the first page table
    vm_mngr_free_frame(cur_pd_addr);

    // Delete the page directory entry
    page_del_attrib(cur_pd_addr, PAGE_PRESENT);
    
    // Flush the TLB
    for (int i = 0, va = 0; i < 256; i++, va += 4096)
        __asm__ volatile ("invlpg (%0);" :: "r"(va));
}

void
vm_mngr_higher_kernel_map(uint32_t va, uint32_t pa)
{
    // Pointer to the page directory
    uint32_t *pd = cur_pd_addr;

    // Pointer to the page directory entry (page table directiry is always mapped, NO page fault in accessing it!)
    uint32_t *pde = cur_pd_addr + va_get_dir_index(va);

    int new_page = 0;
    // If the page table is not present, create it!
    if ( !page_is_present(*pde) ) {
        vm_mngr_alloc_frame(pde);
        new_page = 1;
    }
    
    // Get the frame address of the page table
    uint32_t pt_physical_addr = page_get_frame_addr(*pde);

/* Add the mapping to the page table */

    // No page table is mapped! So, to add the mapping from va to pa to it, we need to first map it to the virtual address space with the help of the mapped utility page

    // Pointer to the utility page
    uint32_t *util_pt = (uint32_t*)0xC0100000;

    // Map the page table to 0xFFC00000 so that we can modify it through writing to the virtual address from 0xFFC00000 to 0xFFC01000 (4K)
    page_install_frame_addr(util_pt, pt_physical_addr);
    page_add_attrib(util_pt, PAGE_PRESENT | PAGE_WRITABLE);

    // Pointer to the page table
    uint32_t *pt = (uint32_t*)0xFFC00000;
    // Clear the newly created page table
    if (new_page)
        mem_set((char*)pt, 0, BLOCK_SIZE);

    // Pointer to the page table entry
    uint32_t *pte = pt + va_get_page_index(va);

    // Install the page table entry
    page_install_frame_addr(pte, pa);
    page_add_attrib(pte, PAGE_PRESENT | PAGE_WRITABLE);

    // Clear the utility page (umapping the page table)
    *util_pt = 0;
}

void
vm_mngr_higher_kernel_unmap(uint32_t va)
{
    // Pointer to the page directory
    uint32_t *pd = cur_pd_addr;

    // Pointer to the page directory entry (page table directiry is always mapped, NO page fault in accessing it!)
    uint32_t *pde = cur_pd_addr + va_get_dir_index(va);

    // If page table is not present, return
    if ( !page_is_present(*pde) )
        return;

    // Get the frame address of the page table
    uint32_t pt_physical_addr = page_get_frame_addr(*pde);

/* Deleting the mapping from the page table */
    // Map the page table to 0xFFC00000 so that we can modify it through writing to the virtual address from 0xFFC00000 to 0xFFC01000 (4K)

    // Pointer to the utility page
    uint32_t *util_pt = (uint32_t*)0xC0100000;
    // Map
    page_install_frame_addr(util_pt, pt_physical_addr);
    page_add_attrib(util_pt, PAGE_PRESENT | PAGE_WRITABLE);

    // Pointer to the page table
    uint32_t *pt = (uint32_t*)0xFFC00000;

    // Pointer to the page table entry
    uint32_t *pte = pt + va_get_page_index(va);

    // Delete the page table entry
    page_del_attrib(pte, PAGE_PRESENT);

    // Scan the page table, if no pages present, remove the page table
    for (; (uint32_t)pt < 0xFFC01000; pt++)
        if ( page_is_present(*pt) )
            break;
    
    if ((uint32_t)pt == 0xFFC01000)
        vm_mngr_free_frame(pde);

    // Clear the utility page (umapping the page table)
    *util_pt = 0;
}