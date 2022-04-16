#include"hd.h"

void ata_read_sectors(uint32_t lba, uint8_t* buf, uint8_t sectorct)
{
    // LBA is 28 bits
    if (lba & 0xF0000000)
        return;

    // Specify how many sectors to read
    port_byte_out(ATA_PORT_SECT_CT, sectorct);

    // Send 28bit LBA byte by byte from port 0x1f3 to 0x1f7
    port_byte_out(ATA_PORT_LBA_0_7, (uint8_t)lba);
    port_byte_out(ATA_PORT_LBA_8_15, (uint8_t)(lba >> 8));
    port_byte_out(ATA_PORT_LBA_16_23, (uint8_t)(lba >> 16));
    port_byte_out(ATA_PORT_LBA_24_28_AND_MODE, (uint8_t)(lba >> 24 | 0xE0)); // Higer 4 bits is data transfer mode, lower 4 bits is LBA bit 24-28
    // 0xE for LBA mode

    // Send command : read with retry
    port_byte_out(ATA_PORT_CMD, ATA_CMD_READ);

    uint8_t status;

    // Test if the disk/ controller is ready
    // by iteratively checking the status - 4th bit being 1 -> ready, 7th bit being one -> not ready
    while (status = port_byte_in(ATA_PORT_CMD) & 0x80); // Waits if not ready

    // Error
    if (status & 0x01)
        return;

    for (int i = 0; i < sectorct; i++) {
        for (int j = 0; j < 256; j++, buf += 2)
            *((uint16_t*)buf) = port_word_in(ATA_PORT_DATA); // Cast it to a pointer that points to 16bit data
        // Test if the disk/ controller is ready
        // by iteratively checking the status - 4th bit being 1 -> ready, 7th bit being one -> not ready
        while (status = port_byte_in(ATA_PORT_CMD) & 0x80); // Waits if not ready

        // Error
        if (status & 0x01)
            return;
    }
}

void ata_write_sectors(uint8_t* buf, uint32_t lba, uint8_t sectorct)
{
    // LBA is 28 bits
    if (lba & 0xF0000000)
        return;
    
    port_byte_out(ATA_PORT_SECT_CT, sectorct);

    port_byte_out(ATA_PORT_LBA_0_7, (uint8_t)lba);
    port_byte_out(ATA_PORT_LBA_8_15, (uint8_t)(lba >> 8));
    port_byte_out(ATA_PORT_LBA_16_23, (uint8_t)(lba >> 16));
    port_byte_out(ATA_PORT_LBA_24_28_AND_MODE, (uint8_t)(lba >> 24 | 0xE0));

    port_byte_out(ATA_PORT_CMD, ATA_CMD_WRITE);

    uint8_t status;

    while (status = port_byte_in(ATA_PORT_CMD) & 0x80); // Wait until ready

    // Error
    if (status & 0x01)
        return;

    for (int i = 0; i < sectorct; i++) {
        for (int j = 0; j < 256; j++, buf += 2)
            port_word_out(ATA_PORT_DATA, *((uint16_t*)buf));
        // Test if the disk/ controller is ready
        // by iteratively checking the status - 4th bit being 1 -> ready, 7th bit being one -> not ready
        while (status = port_byte_in(ATA_PORT_CMD) & 0x80); // Waits if not ready

        // Error
        if (status & 0x01)
            return;
    }
}

void ata_flush()
{
    port_byte_out(ATA_PORT_CMD, ATA_CMD_FLUSH);

    uint8_t status;

    while (status = port_byte_in(0x1F7) & 0x80);

    // Error
    if (status & 0x01)
        ;
}