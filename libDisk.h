#ifndef LIBDISK_H
#define LIBDISK_H

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#define BLOCK_SIZE 256

int openDisk(char *filename, int nBytes);
int closeDisk(int disk);
int readBlock(int disk, int bNum, void *block);
int writeBlock(int disk, int bNum, void *block);

#endif /* LIBDISK_H */