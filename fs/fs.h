#ifndef FS_H
#define FS_H

#include"iocache.h"

#include"../kernel/kmalloc.h"

#include"../Lib/util.h"

#include"../Lib/string.h"

#include"iocache.h"

/**
 * 
 * We implement FAT file system here
 * 
 * Assuming the disk size is 512M, with 4K block size,
 * the FAT should be of 128K entries. Assuming the root
 * directory also contains 512 entries, knowing each entry
 * is 32 bytes, then the root directory would have a size
 * of 512 * 32 = 2^9 * 2^5 = 2^14 = 4 * 2^12 = 4 * 0x1000 (4K)
 * = 16K
 * 
 * Disk partition: 
 *          
 *              || MBR(512B)|| OS(512K) || 512B * 7 (not used, first byte is magic byte) || FAT(512K) || ROOT(16K) || FREE ||
 *              0                                                                        516K         1028K        1044K
 * 
 */

/**
 * Initializing the hard dive and creating the aforemetioned divisions
 */

void fs_init();
void fs_read();
void fs_add_dir(uint32_t dirblockno, char *name);
uint32_t fs_find(uint32_t dirblockno, char *path);
uint32_t get_free_blocks(uint32_t ct);
#endif