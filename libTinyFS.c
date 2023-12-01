#include "libTinyFS.h"

int tfs_mkfs(char *filename, int nBytes){
    int res = 0;
    int finalRes = 0;
    int numBlocks = DEFAULT_DISK_SIZE / BLOCKSIZE;

    int fd = openDisk(filename, nBytes);

    // When fd is -1, we are opening a new disk
    if (fd != -1){
        // Init empty buf 0x00
        char *emptyBuf = malloc(BLOCKSIZE * sizeof(char));
        memset(emptyBuf, 0, BLOCKSIZE);

        // Populate disk w/ empty blocks 
        for (int i = 0; i < numBlocks; i++){
            res = writeBlock(fd, i, emptyBuf);
            if (res < 0){
                printf("WRITING_BLOCK_ERROR\n");
                finalRes = -1;
            }
        }
        
        // Create super block
        char *superBlockBuf = malloc(BLOCKSIZE * sizeof(char));
        superBlockBuf[0] = SUPER_BLOCK; 
        superBlockBuf[1] = MAGIC_NUM;

        char freeBlockArr[numBlocks];
        memset(freeBlockArr, 'F', numBlocks);
        memcpy(superBlockBuf + 2, freeBlockArr, numBlocks);

        res = writeBlock(fd, 0, superBlockBuf);
        if (res < 0){
            printf("WRITING_BLOCK_ERROR\n");
            finalRes = -1;
        }

        free(emptyBuf);
    } else {
        printf("OPEN_DISK_ERROR\n");
        finalRes = -1;
    }

    return finalRes;
}