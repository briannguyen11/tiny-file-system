#ifndef LIBTINYFS_H
#define LIBTINYFS_H

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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
    uint8_t numBlocks;
    char dMap[BLOCKSIZE - 3];
} SuperBlock;

typedef struct InodeBlock {
    char type;
    char mNum;
    char filename[9];
    uint16_t fp;
    uint16_t fSize;
    uint8_t fcbLen;
    uint8_t posInDsk;
    char createTime;
    char modTime;
    char accessTime;
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
int tfs_deleteFile(fileDescriptor fd);
int tfs_readByte(fileDescriptor fd, char *buffer);
int tfs_seek(fileDescriptor fd, int offset);

/* Additional Functionality */
int tfs_rename(fileDescriptor fd, char* newName);
int tfs_readdir();
int tfs_displayFragments();
int tfs_readFileInfo(fileDescriptor fd);

/* Helper Functions */
int setupFS(int diskFd, int numBlocks);
int removeInAndFcb(int diskFd, char *filename);
int getStartBlock(int wrBlockSize, char dMap[], int numBlocks);
#endif /* LIBTINYFS_H*/