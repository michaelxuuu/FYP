#include"iocache.h"

cache* iocache;

// Initialize the io cache
void iocache_init()
{
    iocache = (cache*)kmalloc(sizeof(cache));
    iocache->head->prev = iocache->head->next = iocache->head = iocache->bufs;
    iocache->head->refct = 0;

    buf *b = iocache->bufs + 1;
    for (; b < iocache->bufs + BUF_NO; b++)
    {
        b->refct = 0;
        iocache->head->prev = b;
        b->next = iocache->head;
        b->prev = b;
        iocache->head = b;
    }
    b--;
    b->prev = iocache->bufs;
    iocache->bufs->next = b;
}

buf* bget(uint32_t blockno)
{
    buf *b = iocache->bufs;

    // Is the block already cached?
    for (b = iocache->head; b != iocache->head->next; b = b->prev)
        // Cached: set head to 'b', and return head
        if (b->blockno == blockno) 
        {
            b->refct++;
            return b;
        }
    if (b->blockno == blockno)
    {
            b->refct++;
            return b;
    }
    // Not cached? Recycle the least recently used buffer not in use to buffer the block requested
    for (b = iocache->head->next; b != iocache->head; b = b->prev)
        if (!b->refct) // Not in use
        {
            b->blockno = blockno;
            b->refct = 1;
            b->valid = 0;
            return b;
        }
    if (!b->refct) // Not in use
    {
        b->blockno = blockno;
        b->refct = 1;
        b->valid = 0;
        return b;
    }
    // Out of buffers -> panic
    return 0;
}

buf* bread(uint32_t blockno)
{   
    buf *b = bget(blockno);
    if(!b->valid) {
        ata_read_sectors((b->blockno) * 8, b->data, 8);
        b->valid = 1;
    }
    return b;
}

void bwrite(buf *b)
{
  ata_write_sectors(b->data, (b->blockno) * 8, 8);
}

void brelease(buf *b)
{
    b->refct--;

    if (b == iocache->head)
        return;

    // b->prev <=> b->next
    b->prev->next = b->next;
    b->next->prev = b->prev;

    // head->prev <=> b <=> head
    iocache->head->prev->next = b;
    b->prev = iocache->head->prev;
    iocache->head->prev = b;
    b->next = iocache->head;

    // Set 'b' as head
    iocache->head = b;
}