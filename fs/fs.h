#ifndef FS_H
#define FS_H

/**
 * 
 * We implement FAT file system here
 * 
 * Assuming the disk size is 512M, with 4K block size,
 * the FAT should be of 
 * 
 */

/**
 * Initializing the hard dive and creating the aforemetioned divisions
 */
void fs_init();

#endif