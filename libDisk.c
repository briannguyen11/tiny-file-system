#include "libDisk.h"

/**
 * Description: Create a new disk with inital allocated
 *              space if disk does not already exist. If disk
 *              exist, then just open.
 * Params: Filename and nBytes
 * Return: Valid file descriptor or -1 indicating error
 */
int openDisk(char *filename, int nBytes) {
    int fd;
    if (nBytes == 0) {
        // if filename does not exit, return -1 otherwise file exists
        fd = open(filename, O_RDWR);
    } else if (filename == NULL || nBytes < BLOCKSIZE) {
        fd = -1;
    } else {
        // create a new disk file
        fd = open(filename, O_RDWR | O_CREAT | O_TRUNC,
                  0660);  // enable RW for owner, groups, others
    }
    return fd;
}

/**
 * Description: Close disk file descriptor
 * Params: Disk (file descriptor)
 * Return: 0 for sucess or -1 indicating error
 */
int closeDisk(int disk) {
    if (close(disk) == -1) {
        return -1;
    } else {
        return 0;
    }
}

/**
 * Description: Read from disk into local buf.
 *              Block to read is determined from bNum offset.
 * Params: Disk, bNum (block number), block (pointer to buf)
 * Return: Res of 0 for sucess or -1 indicating error
 */
int readBlock(int disk, int bNum, void *block) {
    int res = 0;
    // Check if disk is valid or bNum is negative
    if (fcntl(disk, F_GETFD) == -1 || bNum < 0) {
        res = -1;
    }
    // Check if buffer exists
    else if (block == NULL) {
        res = -1;
    }
    // Check if lseek is successful (lseek moves fd over offset amount)
    else if (lseek(disk, bNum * BLOCKSIZE, SEEK_SET) == -1) {
        res = -1;
    }
    // Read the block
    else {
        if (read(disk, block, BLOCKSIZE) == -1) {
            res = -1;
        }
    }
    return res;
}

/**
 * Description: Write from local buf into disk.
 *              Block to write to is determined from
 *              bNum offset.
 * Params: Disk, bNum (block number), block (pointer to buf)
 * Return: Res of 0 for sucess or -1 indicating error
 */
int writeBlock(int disk, int bNum, void *block) {
    int res = 0;
    // Check if disk is valid or bNum is negative
    if (fcntl(disk, F_GETFD) == -1 || bNum < 0) {
        res = -1;
    }
    // Check if buffer exists
    else if (block == NULL) {
        res = -1;
    }
    // Check if lseek is successful (lseek moves fd over offset amount)
    else if (lseek(disk, bNum * BLOCKSIZE, SEEK_SET) == -1) {
        res = -1;
    }
    // Write to the block
    else {
        if (write(disk, block, BLOCKSIZE) == -1) {
            res = -1;
        }
    }
    return res;
}
