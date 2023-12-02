#ifndef LIBTINYFS_H
#define LIBTINYFS_H

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

/* Helper Functions */
int setupFileSystem(int fd, int numBlocks);

#endif /* LIBTINYFS_H*/