#ifndef FS_H
#define FS_H

#include"iocache.h"

#include"../kernel/kmalloc.h"

#include"../Lib/util.h"

#include"../Lib/string.h"

#include"iocache.h"

#define MAX_BLOCK 0x20000  // 512M/4K  = 128K
#define MAX_SECTR 0x100000 // 512M/512 = 1M

#define MAGIC_SECTNO 1025
#define FAT_SECTNO 1032
#define ROOTDIR_SECTNO 2056
#define USABLE_SECTNO 2088

#define FAT_BLOCKNO 129
#define ROOTDIR_BLCOKNO 257
#define USABLE_BLOCKNO 261

#define DIRENT_ATTRIB_USED 0x1
#define DIRENT_ATTRIB_USER 0x2
#define DIRENT_ATTRIB_DIR 0x4
#define DIRENT_ATTRIB_DEVICE 0x8

#define DIRENT_PER_BLOCK 128 // 4K/32

typedef struct dirent
{

    uint8_t name[19];
    uint8_t attrib;
    uint32_t blockno;
    uint32_t size;
    uint32_t EOF;

} __attribute__((packed)) dirent;

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

void dirent_set_name(dirent *e, char *name);

void dirent_set_attrib(dirent *e, uint8_t attrib);

void dirent_set_blockno(dirent *e, uint32_t blockno);

void dirent_set_size(dirent *e, uint32_t size);

extern dirent sys_root_dir;

/* -------------------------------Funstions that help manage the File Allocation Table------------------------------- */
void fat_set_next(uint32_t blockno, uint32_t next);

uint32_t fat_get_next(uint32_t blockno);

void fs_init();
/* -------------------------------Funstions that help manage the free space------------------------------- */
uint32_t get_first_free_block();

uint32_t get_free_size();

void set_first_free_block(uint32_t blockno);

void set_free_size(uint32_t size);

// Reduce the number of free space blocks by 'ct' (by 'advancing' the starting block of the free space)
// Returns the prior starting block of the free space
uint32_t get_free_blocks(uint32_t ct);

dirent* dir_lookup(dirent *d, char *name, uint8_t attrib);

// Takes the path and recursively locate the file starting from the given diretory 'start_dir'
// Returns the starting block of the file
dirent* fs_find_in(dirent *d, char *path, uint8_t attrib);

// Add an entry of another directory to the specified directory
int fs_add_dir_at(dirent* d, char *name);

dirent* fs_find(char *p, uint8_t attrib);

int fs_add_file_at(dirent* d, char *name, uint8_t attrib, int size);

void fs_read();

#endif