#include "libTinyFS.h"

int tfs_mkfs(char *filename, int nBytes) {
    int fd = 0;
    int tmp = 0;
    int numBlocks = nBytes / BLOCKSIZE;

    if ((fd = openDisk(filename, nBytes)) < 0) {
        printf("Error: Failed to open disk in function: tfs_mkfs\n");
        return TFS_MKFS_FAIL;
    } else {
        // init empty buf (0x00)
        char *emptyBuf = calloc(BLOCKSIZE, sizeof(char));

        // populate disk w/ empty buf
        for (int i = 0; i < numBlocks; i++) {
            tmp = writeBlock(fd, i, emptyBuf);
            if (tmp < 0) {
                printf("Error: Failed to write in function: tfs_mkfs\n");
                return TFS_MKFS_FAIL;
            }
        }
        free(emptyBuf);

        // init super block
        SuperBlock superBlock;
        superBlock.type = 1;
        superBlock.mNum = 0x44;

        char freeBlockArr[numBlocks];          // creating free block bit vector
        memset(freeBlockArr, 'F', numBlocks);  // init bit vector to all free
        freeBlockArr[0] = 'T';  // set first block to taken by super block

        memset(superBlock.data, 0, sizeof(superBlock.data));
        memcpy(superBlock.data, freeBlockArr, numBlocks);

        tmp = writeBlock(fd, 0, &superBlock);
        if (tmp < 0) {
            printf("Error: Failed to write in function: tfs_mkfs\n");
            return TFS_MKFS_FAIL;
        }
    }

    return TFS_MKFS_SUCCESS;
}
