#ifndef BUF_H
#define BUF_H

#include"../include/type.h"

#include"../kernel/kmalloc.h"

#include"../kernel/kprintf.h"

#include"../drivers/hd.h"

#define BUF_SIZE        4096
#define BUF_NO          32

typedef struct iobuf buf;

struct iobuf
{
    int     blockno;
    char    data[BUF_SIZE];
    int     refct;
    int     valid;
    buf     *next;
    buf     *prev;
};

typedef struct iocache
{
    buf bufs[BUF_NO];
    buf *head;
} cache;

extern cache* iocache;

// Initialize the io cache
void iocache_init();

void make_most_recent(buf *b);

buf* bget(uint32_t blockno);

buf* bread(uint32_t blockno);

void bwrite(buf *b);

void brelease(buf *b);

#endif