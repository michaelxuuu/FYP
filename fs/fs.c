#include "fs.h"

/**
 *
 *          || MBR(512B)|| OS(512K) || 512B * 7 (not used, first byte is magic byte) || FAT(512K) || ROOT(16K) || USABLE ||
 * size     0                                                                        516K         1028K         1044K
 * sect     0           1           1025                                             1032         2056          2088
 * block    0           NONE        NONE                                             129          257           261
 *
 */

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

    uint8_t name[23];
    uint8_t attrib;
    uint32_t blockno;
    uint32_t size;

} __attribute__((packed)) dirent;

/* -------------------------------Funstions that help manage the Directory Entries------------------------------- */
void dirent_set_name(dirent *e, char *name)
{
    mem_copy(name, e->name, str_len(name));
}

void dirent_set_attrib(dirent *e, uint8_t attrib)
{
    e->attrib = attrib;
}

void dirent_set_blockno(dirent *e, uint32_t blockno)
{
    e->blockno = blockno;
}

void dirent_set_size(dirent *e, uint32_t size)
{
    e->size = size;
}

dirent sys_root_dir;

dirent cur_dir;

/* -------------------------------Funstions that help manage the File Allocation Table------------------------------- */
void fat_set_next(uint32_t blockno, uint32_t next)
{
    buf *b = bread(FAT_BLOCKNO + blockno / 1024);
    ((uint32_t *)b->data)[blockno % 1024] = next;
    bwrite(b);
    brelease(b);
}

uint32_t fat_get_next(uint32_t blockno)
{
    buf *b = bread(FAT_BLOCKNO + blockno / 1024);
    uint32_t next = ((uint32_t *)b->data)[blockno % 1024];
    brelease(b);
    return next;
}

void fs_init()
{
    // Check magic, file system exists already if magic byte being 0xFF
    // If there isn't a file system, we set it up
    char magic[512];
    ata_read_sectors(MAGIC_SECTNO, magic, 1);
    if (magic[0])
        return;

    // Create a file system
    // Create file allocation table and flush it to the hard drive one block

    // Create File Allocation Table and flush it to the hard drive one block at a time
    for (int blockno = FAT_BLOCKNO; blockno < ROOTDIR_BLCOKNO; blockno++)
    {
        buf *b = bget(blockno); // get buffer
        for (int i = 0; i < 1024; i++)
            ((uint32_t *)(b->data))[i] = (blockno - FAT_BLOCKNO) * 1024 + i + 1; // write buffer
        if (blockno == ROOTDIR_BLCOKNO - 1)                                      // EOF of free block
            ((uint32_t *)(b->data))[1023] = 0;
        bwrite(b); // flush buffer
        brelease(b);
    }

    // Create FAT root diretory and system root directory
    for (int blockno = ROOTDIR_BLCOKNO; blockno < USABLE_BLOCKNO; blockno++)
    {
        buf *b = bread(blockno);
        mem_set(b->data, 0, 4096);
        dirent *rootdir = (dirent *)b->data;
        if (blockno == ROOTDIR_BLCOKNO)
        {
            // File for tracking the unallocated blocks
            dirent_set_name(rootdir, "free");
            dirent_set_attrib(rootdir, DIRENT_ATTRIB_USED);
            dirent_set_blockno(rootdir, USABLE_BLOCKNO);
            dirent_set_size(rootdir, MAX_BLOCK - USABLE_BLOCKNO);
            bwrite(b);

            // System's root diretcory
            dirent_set_name(rootdir + 1, "root");
            dirent_set_attrib(rootdir + 1, DIRENT_ATTRIB_USED);
            uint32_t freeblock = get_free_blocks(1);
            fat_set_next(freeblock + 1, 0);
            dirent_set_blockno(rootdir + 1, freeblock);
            dirent_set_size(rootdir + 1, 4);

            // Install the system root diretcory
            dirent_set_name(&sys_root_dir, "root");
            dirent_set_attrib(&sys_root_dir, DIRENT_ATTRIB_USED);
            dirent_set_blockno(&sys_root_dir, freeblock);
            dirent_set_size(&sys_root_dir, 4);

            // Set the current directory to the system root directory
            cur_dir = sys_root_dir; 
        }
        bwrite(b);
        brelease(b);
    }

    // Set  magic byte
    magic[0] = 0xff;
    ata_write_sectors(magic, MAGIC_SECTNO, 1);
}

/* -------------------------------Funstions that help manage the free space------------------------------- */
uint32_t get_first_free_block()
{
    buf *b = bread(ROOTDIR_BLCOKNO);
    uint32_t blockno = ((dirent *)b->data)[0].blockno;
    brelease(b);
    return blockno;
}

uint32_t get_free_size()
{
    buf *b = bread(ROOTDIR_BLCOKNO);
    uint32_t size = ((dirent *)b->data)[0].size;
    brelease(b);
    return size;
}

void set_first_free_block(uint32_t blockno)
{
    buf *b = bread(ROOTDIR_BLCOKNO);
    ((dirent *)b->data)[0].blockno = blockno;
    bwrite(b);
    brelease(b);
}

void set_free_size(uint32_t size)
{
    buf *b = bread(ROOTDIR_BLCOKNO);
    ((dirent *)b->data)[0].size = size;
    bwrite(b);
    brelease(b);
}

// Reduce the number of free space blocks by 'ct' (by 'advancing' the starting block of the free space)
// Returns the prior starting block of the free space
uint32_t get_free_blocks(uint32_t ct)
{
    /* Get the starting block of the free space */
    uint32_t prior = get_first_free_block();

    /* Advance the starting block of the free space for 'ct' times */
    uint32_t next = prior;
    for (int i = 0; i < ct; i++)
        next = fat_get_next(next);

    set_first_free_block(fat_get_next(next));

    return prior;
}

uint32_t dir_lookup(uint32_t dirblockno, char *name)
{
    buf *b = bread(dirblockno);

    dirent *p = (dirent *)b->data;

    uint32_t blockno = 0;

    int len = str_len(name);

    int i = 0;
    for (; i < DIRENT_PER_BLOCK; i++)
        if (str_cmp(p[i].name, name, len) == 0)
        {
            blockno = p[i].blockno;
            break;
        }

    brelease(b);

    return blockno;
}

// Takes the path and recursively locate the file starting from the given diretory 'start_dir'
// Returns the starting block of the file
uint32_t fs_find(uint32_t dirblockno, char *path)
{
    int len = str_len(path);
    char *p = kmalloc(len);
    mem_copy(path, p, len + 1);

    // replace the slashes with 0s automatically breaking up the path into individual strings
    for (int i = 0; i < len - 1; i++)
        if (p[i] == '/')
            p[i] = 0;

    // current directory
    uint32_t cur_dir_blockn = dirblockno;

    for (char *name = p; name < p + len; name += str_len(name))
    {
        name += 1; // next name

        cur_dir_blockn = dir_lookup(cur_dir_blockn, name);

        if (!cur_dir_blockn)
        {
            kfree(p);
            return 0;
        }
    }

    kfree(p);

    return cur_dir_blockn;
}

// Add an entry of another directory to the specified directory
void fs_add_dir(uint32_t dirblockno, char *name)
{
    buf *b = bread(dirblockno);
    dirent *p = (dirent *)b->data;

    int i = 0;
    for (; i < DIRENT_PER_BLOCK; i++)
    {
        if ((p[i].attrib & DIRENT_ATTRIB_USED) == 0)
            break;
    }

    if (i != DIRENT_PER_BLOCK)
    {
        p += i;
        dirent_set_name(p, name);
        dirent_set_attrib(p, DIRENT_ATTRIB_USED);
        uint32_t free_blockno = get_free_blocks(1);
        dirent_set_blockno(p, free_blockno);
        fat_set_next(free_blockno, 0);
        dirent_set_size(p, 4); // Diretcories are all 4K in size containing 32 32-byte directory entries

        bwrite(b);
        brelease(b);

        // Clear the buffer and add '.' and '..' to the newly created directory
        b = bget(free_blockno);
        p = (dirent *)b->data;
        mem_set((char*)p, 0, 4096);

        dirent_set_name(p, ".");
        dirent_set_attrib(p, DIRENT_ATTRIB_USED);
        dirent_set_blockno(p, free_blockno);
        dirent_set_size(p, 4);

        p++;
        dirent_set_name(p, "..");
        dirent_set_attrib(p, cur_dir.attrib);
        dirent_set_blockno(p, cur_dir.blockno);
        dirent_set_size(p, cur_dir.size);

        bwrite(b);
        brelease(b);
    }
    brelease(b);
}

uint32_t fs_find_(char *p)
{
    uint32_t bn = fs_find(cur_dir.blockno, p);

    if (bn)
        return bn;
    
    bn = fs_find(sys_root_dir.blockno, p);

    return bn ? bn : 0;
}

void fs_read()
{
    fs_add_dir(sys_root_dir.blockno, "testdir");
    uint32_t blo = fs_find_("/testdir");
    fs_add_dir(blo, "test_subdir");
    blo = fs_find_("/test/test_subdir");
    fs_add_dir(blo, "deepestdir");
    buf *b = bread(blo);
}