#ifndef LIBTINYFS_H
#define LIBTINYFS_H

#include <fcntl.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include "TinyFS_errno.h"
#include "libDisk.h"
#include "tinyFS.h"

typedef struct FileEntry {
    fileDescriptor fd;       // fd of open file
    char filename[9];        // filename only 8 characters
    struct FileEntry *next;  // LL to be dynamic
    time_t initTime;
} FileEntry;

typedef struct SuperBlock {
    char type;                 // 1
    char mNum;                 // 0x44
    uint8_t numBlocks;         // num of blocks in disk
    char dMap[BLOCKSIZE - 3];  // map of disk (S,I,F,C for block types)
} SuperBlock;

typedef struct InodeBlock {
    char type;
    char mNum;
    char filename[9];
    uint16_t fp;
    uint16_t fSize;
    uint8_t fcbLen;
    uint8_t posInDsk;
    uint8_t rdOnly;
    time_t createTime;
    time_t modTime;
    time_t accessTime;
    char data[BLOCKSIZE - 21];
} InodeBlock;

typedef struct FileContextBlock {
    char type;                    // 3
    char mNum;                    // 0x44
    char context[BLOCKSIZE - 2];  // file content
} FileContextBlock;

typedef struct FreeBlock {
    char type;                 // 4
    char mNum;                 // 0x44
    char data[BLOCKSIZE - 2];  // all 0x00
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
int tfs_defrag();
int tfs_rename(fileDescriptor fd, char *newName);
int tfs_readdir();
int tfs_displayFragments();
int tfs_readFileInfo(fileDescriptor fd);
int tfs_makeRO(char *name);
int tfs_makeRW(char *name);
int tfs_writeByte(fileDescriptor fd, uint8_t data);

/* Helper Functions */
int setupFS(int diskFd, int numBlocks);
int removeInAndFcb(int diskFd, char *filename);
int getStartBlock(int wrBlockSize, char dMap[], int numBlocks);
#endif /* LIBTINYFS_H*/