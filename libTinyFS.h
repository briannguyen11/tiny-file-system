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
    char dMap[BLOCKSIZE - 2];
} SuperBlock;

typedef struct InodeBlock {
    char type;
    char mNum;
    char filename[9];
    char index;
    char size;
    char numCtxBlock;
    char data[BLOCKSIZE - 14];
} InodeBlock;

typedef struct FileContextBlock {
    char type;
    char mNum;
    char context[BLOCKSIZE - 2];
} FileContextBlock;

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
int getStartBlock(int diskFd, int wrBlockSize, char fbArr[]);
#endif /* LIBTINYFS_H*/