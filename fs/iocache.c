#include"iocache.h"

#define BUF_SIZE    1024
#define BUF_NO      32

typedef struct iobuf buf;

struct iobuf
{
    int     blockno;
    char    data[BUF_SIZE];
    buf     *next;
    buf     *prev;
};

typedef struct iocache
{
    buf bufs[BUF_NO];
    buf *head;
} cache;

// Initialize the io cache
void iocache_init()
{
    cache *c = (cache*)kmalloc(sizeof(cache));
    c->head->prev = c->head->next = c->head = c->bufs;

    buf *b = c->bufs + 1;
    for (; b < c->bufs + BUF_NO; b++)
    {
        c->head->prev = b;
        b->next = c->head;
        b->prev = b;
        c->head = b;
    }
    b--;
    b->prev = c->bufs;
    c->bufs->next = b;

    for (int i = 0; i <32; i++)
    {
        (c->bufs[i]).data[0] = i;
    }

    for (b = c->head; b != c->bufs; b = b->next)
    {
        printf("%d ", b->data[0]);
    }
}
