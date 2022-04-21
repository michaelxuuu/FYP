#include "fs.h"

/**
 *
 *          || MBR(512B)|| OS(512K) || 512B * 7 (not used, first byte is magic byte) || FAT(512K) || ROOT(16K) || USABLE ||
 * size     0                                                                        516K         1028K         1044K
 * sect     0           1           1025                                             1032         2056          2088
 * block    0           NONE        NONE                                             129          257           261
 *
 */

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
        }
        bwrite(b);
        brelease(b);
    }

    // Set  magic byte
    magic[0] = 0xff;
    ata_write_sectors(magic, MAGIC_SECTNO, 1);

    // add a file
    fs_add_file_at(&sys_root_dir, "shell.bin", DIRENT_ATTRIB_USED);
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

dirent* dir_lookup(dirent *d, char *name, uint8_t attrib)
{
    buf *b = bread(d->blockno);

    dirent *p = (dirent *)b->data;

    int i = 0;
    for (; i < DIRENT_PER_BLOCK; i++)
    {
        if (str_len(p[i].name) == str_len(name))
            if (str_cmp(p[i].name, name, str_len(name)) == 0 && p[i].attrib == attrib)
                break;
    }

    brelease(b);

    if (i != DIRENT_PER_BLOCK)
    {
        dirent *f = (dirent*)kmalloc(sizeof(dirent));
        *f = p[i];
        return f;
    }

    return 0;
}

// Takes the path and recursively locate the file starting from the given diretory 'start_dir'
// Returns the starting block of the file
dirent* fs_find_in(dirent *d, char *path, uint8_t attrib)
{
    int len = str_len(path);
    char *p = kmalloc(len);
    mem_copy(path, p, len + 1);

    // replace the slashes with 0s automatically breaking up the path into individual strings
    for (int i = 0; i < len - 1; i++)
        if (p[i] == '/')
            p[i] = 0;

    // current directory
    dirent* cur_dir = d;

    for (char *name = p; name < p + len; name += str_len(name))
    {
        name += 1; // next name

        cur_dir = dir_lookup(cur_dir, name, attrib);

        if (!cur_dir)
        {
            kfree(p);
            return 0;
        }
    }

    kfree(p);

    return cur_dir;
}

// Add an entry of another directory to the specified directory
void fs_add_dir_at(dirent* d, char *name)
{
    buf *b = bread(d->blockno);
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
        dirent_set_attrib(p, DIRENT_ATTRIB_USED | DIRENT_ATTRIB_DIR);
        dirent_set_blockno(p, free_blockno);
        dirent_set_size(p, 4);

        p++;
        dirent_set_name(p, "..");
        dirent_set_attrib(p, d->attrib);
        dirent_set_blockno(p, d->blockno);
        dirent_set_size(p, d->size);

        bwrite(b);
        brelease(b);
    }
    brelease(b);
}

// Add an empty file to the specified directory (file fixed to 4K in size)
void fs_add_file_at(dirent* d, char *name, uint8_t attrib)
{
    buf *b = bread(d->blockno);
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
    }
    bwrite(b);
    brelease(b);
}

dirent* fs_find(char *p, uint8_t attrib)
{
    return fs_find_in(&sys_root_dir, p, attrib);
}

// void fs_read()
// {
//     fs_add_dir_at(&sys_root_dir, "test");
//     dirent* d = fs_find("/test");
//     fs_add_dir_at(d, "test_sub");
//     d = fs_find("/test/test_sub");
//     fs_add_dir_at(d, "deepestdir");
//     buf *b = bread(d->blockno); // gdb: p b->data 
// }