#include "libTinyFS.h"

// Global Variables
char *mDisk = NULL;  // mounted disk

/**
 * Opens a new disk and initializes it with a super block
 * and free block to store future files
 */
int tfs_mkfs(char *filename, int nBytes) {
    int fd = 0;
    int tmp = 0;
    int numBlocks = nBytes / BLOCKSIZE;

    if ((fd = openDisk(filename, nBytes)) < 0) {
        printf("Error: Failed to open disk in function: tfs_mkfs\n");
        return OPEN_DISK_ERR;
    } else {
        // init empty buf (0x00)
        char *emptyBuf = calloc(BLOCKSIZE, sizeof(char));

        // populate disk w/ empty buf
        for (int i = 0; i < numBlocks; i++) {
            tmp = writeBlock(fd, i, emptyBuf);
            if (tmp < 0) {
                printf("Error: Failed to write in function: tfs_mkfs\n");
                return WRITE_BLOCK_ERR;
            }
        }
        free(emptyBuf);

        // setup file system with super block and free blocks
        tmp = setupFileSystem(fd, numBlocks);
        if (tmp < 0) {
            printf("Error: Failed to write in function: tfs_mkfs\n");
            return WRITE_BLOCK_ERR;
        }
    }

    return SUCCESS;
}

/**
 * Keeps track of the current disk being accessed
 * for file operations
 */
int tfs_mount(char *diskname) {
    // unmount current disk if applicable
    if (mDisk != NULL) {
        printf("%s is currently mounted. Unmouting now.");
        tfs_unmount();
    }
}

int tfs_unmount(void) {
    free(mDisk);
    if (mDisk == NULL) {
        printf("Nothing to unmount.");
    }
    mDisk = NULL;
    return 0;
}
/*********************** Helper Functions ***********************/

/**
 * Initializes super block and free blocks and writes the blocks
 * into the recently opened disk
 */
int setupFileSystem(int fd, int numBlocks) {
    int tmp = 0;

    /* Init Super Block */
    SuperBlock superBlock;
    superBlock.type = 1;
    superBlock.mNum = 0x44;

    char freeBlockArr[numBlocks];  // creating free block array
    memset(freeBlockArr, 'F',
           numBlocks);      // init free block array to all free
    freeBlockArr[0] = 'T';  // set first block to taken by super block

    memset(superBlock.data, 0, sizeof(superBlock.data));
    memcpy(superBlock.data, freeBlockArr,
           numBlocks);  // copies free block array into super block

    tmp = writeBlock(fd, 0, &superBlock);
    if (tmp < 0) {
        printf("Error: Failed to write in function: setupFileSystems\n");
        return WRITE_BLOCK_ERR;
    }

    /* Init Free Blocks */
    FreeBlock freeBlock;
    freeBlock.type = 4;
    freeBlock.mNum = 0x44;
    memset(freeBlock.data, 0, sizeof(superBlock.data));
    for (int i = 1; i < numBlocks; i++) {
        tmp = writeBlock(fd, i, &freeBlock);
        if (tmp < 0) {
            printf("Error: Failed to write in function: setupFileSystems\n");
            return WRITE_BLOCK_ERR;
        }
    }

    return SUCCESS;
}
