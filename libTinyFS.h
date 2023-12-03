#ifndef LIBTINYFS_H
#define LIBTINYFS_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libDisk.h"
#include "tinyFS.h"
#include "tinyFS_errno.h"

typedef struct FileEntry {
    fileDescriptor fd;
    char filename[9];
    struct FileEntry *next;  // LL to be dynamic
} FileEntry;

typedef struct SuperBlock {
    char type;
    char mNum;
    char data[BLOCKSIZE - 2];
} SuperBlock;

typedef struct InodeBlock {
    char filename[9];
    char size;
    char startBlock;
    char numCtxBlock;
    struct InodeBlock *next;  // LL to be dynamic
} InodeBlock;

typedef struct FreeBlock {
    char type;
    char mNum;
    char data[BLOCKSIZE - 2];
} FreeBlock;

/* Primary Functions */
int tfs_mkfs(char *filename, int nBytes);
int tfs_mount(char *diskname);
int tfs_unmount();
fileDescriptor tfs_openFile(char *name);
int tfs_closeFile(fileDescriptor fd);
int tfs_writeFile(fileDescriptor fd, char *buffer, int size);

/* Helper Functions */
int setupFS(int diskFd);
int getIndexToWrite(int diskFd, int writeBlockSize, char freeBlockArr[]);

#endif /* LIBTINYFS_H*/