#ifndef VM_MNGR_H
#define VM_MNGR_H

#include "../include/type.h"

#include "pm_mngr.h"

#include "../Lib/util.h"

#include "./../cpu/idt.h"

/*===================================================================
 * One way to create the mapping table between the virtual pages
 * and physcial memory blocks is to generate an array of integers
 * where the array index is represents the virtual page number
 * while the value of that integer is the physcial address of the
 * frame address and attributions of that page.
 * 
 *          +----------------------------------------+
 *          |    Frame Addr   (20 bits)  |   Attrib  |
 *          +----------------------------------------+
 *                              .
 *                              .
 *                              .
 *          +----------------------------------------+
 *          |    Frame Addr   (20 bits)  |   Attrib  |
 *          +----------------------------------------+
 *          |    Frame Addr   (20 bits)  |   Attrib  |
 *          +----------------------------------------+
 * 
 * Considering the supported memory by i386 system is 4GB and that
 * one page is 4KB in size, it's easy to calculate that the array
 * needs to contain 1M integers, which consumes 4MB size of memory.
 * 
 * If you got a 4GB memory, then it would not be painful to spare
 * 4MB, but what if the memory size is much smaller, for example,
 * the i386 system emulated by QEMU by default comes with only
 * 128M memory. Therefore, to minimize the cost of storing the
 * mapping table, i386 system adopts a 2-layer mapping scheme,
 * where the first layer is the page directory table and 
 * the second layer are the page tables.
 * 
 * The page directory, which by itself is also a page, 
 * stores 1K entries with each points a page table, then a page 
 * table, which is also a page contains 1K entries, withe each 
 * points to a page, whose physical entity is called a frame in
 * the physical memory that occupies 4KB.
 * 
 * This scheme determined by the system not the software becasue 
 * eventually, the virtual address will be sent to the MMU to be
 * translated into physcial address. How MMU interprets the address
 * is decided by hardware manufactures.
 * 
 * For the i386 system in particular, after the paging is enabled,
 * MMU interpret the virtual in the following way:
 * 
 *          31        22 21        12 11            0
 *          +----------------------------------------+
 *          | dir index | page index |  page offset  |  Linear Addr
 *          +----------------------------------------+
 *                |             |           |                              Frame         
 *                |             |           |                          +----------------------------+         
 *                |             |           +------------------------> | Physical address           |             
 *                |             |                                      |                            |         
 *                |             |                      Page Table      |                            |                       
 *                |             +------------- +     +------------+    +----------------------------+                     
 *                |                            |     |            |           ^ 
 *                |      Page Dir              |     +------------+           |  
 *                |      +------------+        |     |            |           |                 
 *                |      |            |        |     +------------+           |  
 *                |      +------------+        +---> |            | ----------+                           
 *                +--->  |            | ---+         +------------+                   
 *                       +------------+    |         |            |                           
 *                       |            |    |         +------------+                           
 *                       +------------+    |              ^
 *                       |            |    |              |
 *                       +------------+    |              |
 *                                         |              |
 *                                         +--------------+
 * 
 * This way, the entire 4GB address space can be covered with the 
 * advantage that if a page table has no entries, it can be freed 
 * and it's present flag unset in the page directory.
 * 
 * When is a page table to have no entries? A page table has no
 * entries when there isn't enough physical memory to allocate,
 * leaving many page tables unused and whereby having no entries,
 * for example, when the entire physical memory is 128M, which
 * requires at total 128M/4K = 32K pages only and other pages 
 * could all be freed.
 * ===================================================================*/

/**
*  Some functions that help manipulate the 32-bit virtual addresses
*/

/**
 * Retrieve the page directory index given a virtual (linear) address
 */
int va_get_dir_index(uint32_t va);

/**
 * Retrieve the page table index given a virtual (linear) address
 */
int va_get_page_index(uint32_t va);

/**
 * Retrieve the page table offset given a virtual (linear) address
 */
int va_get_page_offset(uint32_t va);

/*===================================================================
 * 
 * 
 * Page table entry format:
 *    
 *    (Refer to 80386 manual page 100 of 421)
 *
 *    Bit
 *    --------------------------------
 *    0
 *        Present (in memory) Flag
 *    0
 *    --------------------------------
 *    1
 *        Read/ Write Flag
 *            0 : Read only
 *            1 : Writable
 *    1
 *    --------------------------------
 *    2 
 *        Operation Mode
 *            0 : Kernel
 *            1 : User 
 *    2
 *    --------------------------------
 *    3
 *        0,0
 *    4
 *    --------------------------------
 *    5
 *        Access Flag (SET BY PROCESSOR)
 *            0 : Page has not been accessed  (read from or written to)
 *            1 : Page has been accessed
 *    5
 *    --------------------------------
 *    6
 *        Dirty Flag (SET BY PROCESSOR)
 *            0 : Page has not been written to
 *            1 : Page has been written to
 *    6
 *    --------------------------------
 *    7
 *        0,0
 *    8
 *    --------------------------------
 *    9
 *        Available for use
 *    11
 *    --------------------------------
 *    12
 *        Frame Address
 *
 *                        Physical memory (4G)
 *            #Frames = ---------------------- = 1M
 *                           Page Size (4K)
 *            
 *            => 20 bits
 *    31
 *    --------------------------------
 * 
 * In conclusion, each page is 4K, each page table has 1K entries
 * and each page directory also has 1K entries. Therefore, 
 * the total virtual memory size is 4KB * 1K * 1K = 4GB. 
 * 
 * ===================================================================*/


/*=================================  PTE  =======================================*/
/**
 * Below provides an abstract interface for the management of Page Table Entries
 */

#define PAGE_PRESENT           0b00000000000000000000000000000001
#define PAGE_WRITABLE          0b00000000000000000000000000000010
#define PAGE_USER              0b00000000000000000000000000000100
#define PAGE_ACCESSED          0b00000000000000000000000000100000 // Not used by page_add_attrib() becasue it is set by the processor, it is used by page_is_accessed()
#define PAGE_DIRTY             0b00000000000000000000000001000000 // Not used by page_add_attrib() becasue it is set by the processor, it is used by page_is_dirty()
#define PAGE_FRAME_ADDR_MASK   0b11111111111111111111000000000000

/**
 * Set certain bits in the PTE to add attributes to a page
 */
void page_add_attrib(uint32_t* ptr_to_pte, int attrib);

/**
 * Clear certain bits in the PTE to delete attributes of a page
 */
void page_del_attrib(uint32_t* ptr_to_pte, int attrib);

/**
 * Install frame addess field in the PTE to assign a physical memory for a page
 */
void page_install_frame_addr(uint32_t* ptr_to_pte, uint32_t frame_addr);

/**
 * Test if the page is present (in the main memory)
 * Returns TRUE if present, or 0 if not
 */
int page_is_present(uint32_t pte);

/**
 * Test if the page is used by the user
 * Returns TRUE if used by the user, or 0 if used by kernel
 */
int page_is_user(uint32_t pte);

/**
 * Test if the page is has been accessed
 * Returns TRUE if accessed, or 0 if not
 */
int page_is_accessed(uint32_t pte);

/**
 * Test if the page is has been marked as dirty
 * Returns TRUE if dirty, or 0 if not
 */
int page_is_dirty(uint32_t pte);

/**
 * Test if the page is has been marked as dirty
 * Returns the physical address of the frame
 */
uint32_t page_get_frame_addr(uint32_t pte);

/*===========================  VIRTUAL MEMORY MANAGER  ==========================*/

#define KERNEL_PD_BASE 0x501000

/**
 * This global veriable does the job of recording the current page directory in use
 * which is stored in the PDBR (Page Directory Base Register - cr3). With this, we
 * can fetch the current page directory address easily without having to read it out
 * from cr3 using a separate routing coded in raw assambly instructions. 
 */

/**
 * Load the page directory address to PDBR
 */
void vm_mngr_load_pd(uint32_t pd_physical_addr);

/**
 * Assign physical memory to a page
 * Returns the frame address of the page if successful or 0 if not
 */
uint32_t vm_mngr_alloc_frame(uint32_t* ptr_to_pte);

/**
 * Free the physical memory allocated for the page
 */
void vm_mngr_free_frame(uint32_t* ptr_to_ptr);

/**
 * Map the physical address to the virtual address on a 4K basis (1 block at a time)
 */
void vm_mngr_map_segmentation(uint32_t pa, uint32_t va);

/**
 * 1. Identity map the kernel so that the execution of the current code won't be affacted when enabling the paging
 * 2. Map the kernel to 3GB virtual to make a higher half kernel
 * 3. Set up the kernel address space
 */
void vm_mngr_init();

void vm_mngr_higher_kernel_map(uint32_t pd_pa, uint32_t va, uint32_t pa, uint32_t attrib);

void vm_mngr_higher_kernel_unmap(uint32_t pd_pa, uint32_t va);

uint32_t* vm_mngr_map_frame(uint32_t pa);

void vm_mngr_unmap_frame(uint32_t *va);

void flush_tlb();

void invalidate_tlb_ent(uint32_t va);

#endif