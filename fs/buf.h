#ifndef BUF_H
#define BUF_H

#include<stdint.h>

/**
 * io buffer list is a linked list of 4KB arrays
 * There are 10 individual buffers in this list
 * which makes the total buffer 40KB, capable of
 * caching 10 blocks from the disk
 * 
 * Each buffer has time-to-live used to determine 
 * which one is the least recently used and will be the
 * replaced in the next 'cache miss'
 */

typedef struct iobuf
{
    uint32_t ttl;
    uint8_t data[4096];
    uint32_t* next;
    uint32_t* prev;
};

// Initialize the buffer
void iobuf_init()
{

}


#endif