#ifndef HD_H
#define HD_H

#include"../include/type.h"

#include"low_level.h"

enum ata_ports 
{
    ATA_PORT_DATA       = 0x1F0,
    ATA_PORT_SECT_CT    = 0x1F2,
    ATA_PORT_LBA_0_7,
    ATA_PORT_LBA_8_15,
    ATA_PORT_LBA_16_23,
    ATA_PORT_LBA_24_28_AND_MODE,
    ATA_PORT_CMD,
};

enum ata_cmds
{
    ATA_CMD_READ    = 0x20,
    ATA_CMD_WRITE   = 0x30,
    ATA_CMD_FLUSH   = 0xE7
};

void ata_read_sectors(uint32_t lba, uint8_t* buf, uint8_t sectorct);

void ata_write_sectors(uint8_t* buf, uint32_t lba, uint8_t sectorct);

void ata_flush();

#endif