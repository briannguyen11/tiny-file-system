#include "libTinyFS.h"

int tfs_mkfs(char *filename, int nBytes){
    int res = 0;
    int finalRes = 0;

    char *emptyBuf;
    // char *superBlockBuf;

    int numBlocks = DEFAULT_DISK_SIZE / BLOCKSIZE;

    int fd = openDisk(filename, nBytes);

    // When fd is -1, we are opening a new disk
    if (fd == -1){
        // Init empty buf 0x00
        emptyBuf = malloc(BLOCKSIZE * sizeof(char));
        memset(emptyBuf, 0, BLOCKSIZE);
        
        // Populate disk with empty buffer
        for (int i = 0; i < numBlocks; i++){
            res = writeBlock(fd, i, emptyBuf);
            if (res < 0){
                printf("WRITING_BLOCK_ERROR\n");
                finalRes = -1;
            }
        }
    } else {
        printf("OPEN_DISK_ERROR\n");
        finalRes = -1;
    }
    return finalRes;
}